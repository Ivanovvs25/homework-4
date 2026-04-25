#include <stdio.h> //fopen, fscanf, printf
#include <stdlib.h> //malloc, calloc, free
#include <windows.h> //SetConsoleOutputCP(866)

typedef struct {
    int** adj_matrix; //указатель на указатели. по сути своей динамический двумерный массив для обращения к элементам матрицы смежности
    int num_vertices; //кол-во вершин
    int num_edges; //кол-во ребер
} Graph;

Graph* createGraph(int vertices, int edges) {
    Graph* g = (Graph*)malloc(sizeof(Graph)); //просим у системы память по нашу структуру
    if (g == NULL) {
        printf("ERROR: Cannot allocate memory for Graph\n"); //обрабока ошибки, если памяти не хватило
        return NULL;
    }

    g->num_vertices = vertices;
    g->num_edges = edges;

    g->adj_matrix = (int**)malloc(vertices * sizeof(int*)); //просим у системы выделить память под массив указателей
    if (g->adj_matrix == NULL) {
        printf("ERROR: Cannot allocate memory for adjacency matrix\n");
        free(g);
        return NULL; //если не удалось выделить память под такой массим, необходимо освободить структуру полностью
    }

    for (int i = 0; i < vertices; i++) {
        g->adj_matrix[i] = (int*)calloc(vertices, sizeof(int)); //выделяем память под каждую строчку, записываем нули
        if (g->adj_matrix[i] == NULL) {
            printf("ERROR: Cannot allocate memory for row %d\n", i); //очистка уже выделенной памяти в случае ошибки
            for (int j = 0; j < i; j++) {
                free(g->adj_matrix[j]);
            }
            free(g->adj_matrix);
            free(g);
            return NULL;
        }
    }

    printf("Graph created: %d vertices, %d edges\n", vertices, edges);
    return g; //возвращаем указатель на структуру
}

Graph* readGraphFromFile(const char* filename) //функция принимает имя файла (строку) и возвращает указатель на готовый граф. Если что-то не так — возвращает NULL.
{
    FILE* file = fopen(filename, "r"); //открытие фалйа  в режиме чтения
    if (file == NULL) {
        printf("ERROR: Cannot open file %s\n", filename);
        return NULL;
    }

    int vertices, edges;
    int result = fscanf(file, "%d %d", &vertices, &edges); //читаем из файла два int числа в формате "%d %d" и помещаем резуьта в переменные int vertices, edges
    if (result != 2) {
        printf("ERROR: Invalid file format (expected two numbers)\n");
        fclose(file);
        return NULL; //если не 2 считанных значения - ошибка
    }

    printf("Vertices: %d\n", vertices);
    printf("Edges: %d\n", edges);

    int** inc_matrix = (int**)malloc(vertices * sizeof(int*)); //Выделяет память под массив указателей на строки матрицы инцидентности
    if (inc_matrix == NULL) {
        printf("ERROR: Cannot allocate memory for incidence matrix\n");
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < vertices; i++) //В цикле для каждой вершины (строки) выделяем память под edges целых чисел
    {
        inc_matrix[i] = (int*)malloc(edges * sizeof(int));
        if (inc_matrix[i] == NULL) {
            printf("ERROR: Cannot allocate memory for row %d of incidence matrix\n", i);
            for (int j = 0; j < i; j++) {
                free(inc_matrix[j]);
            }
            free(inc_matrix);
            fclose(file);
            return NULL;
        }
    }

    printf("INCIDENCE MATRIX:\n");
    printf("     ");
    for (int j = 0; j < edges; j++) // печать стобцов
        {
        printf(" e%-2d", j);
    }
    printf("\n");

    for (int i = 0; i < vertices; i++) //печать строк
        {
        printf("v%d | ", i);
        for (int j = 0; j < edges; j++) {
            if (fscanf(file, "%d", &inc_matrix[i][j]) != 1) //читаем по удному значению из файла и записываем в матрицу
            {
                printf("\nERROR: Insufficient data in file\n");
                for (int k = 0; k < vertices; k++) {
                    free(inc_matrix[k]);
                }
                free(inc_matrix);
                fclose(file);
                return NULL;
            }
            printf("%3d ", inc_matrix[i][j]);
        }
        printf("\n");
    }

    fclose(file);

    Graph* g = createGraph(vertices, edges); //Вызываем createGraph: создание графа с пустой матрицей смежности
    if (g == NULL) //Если создать не удалось — чистим матрицу инцидентности и выходим
      {
        for (int i = 0; i < vertices; i++) {
            free(inc_matrix[i]);
        }
        free(inc_matrix);
        return NULL;
    }

    printf("Converting incidence matrix to adjacency matrix:\n");

    for (int j = 0; j < edges; j++) //Проходим по каждому ребру (столбцу)
        {
        int first_vertex = -1; //1й конец ребра
        int second_vertex = -1; //2й конец ребра
        int is_loop = 0; //флаг на петлю

        for (int i = 0; i < vertices; i++) //Проходим по каждой вершине
            {
            if (inc_matrix[i][j] == 1) //если 1, то
            {
                if (first_vertex == -1) //если первый конец все еще -1, то мы нашли 1 конец
                    {
                    first_vertex = i;
                }
                else //иначе это  второй конец
                {
                    second_vertex = i;
                }
            }
            else if (inc_matrix[i][j] == 2) //если 2 то мы нашли петлю
                {
                is_loop = 1;
                first_vertex = i;
                second_vertex = i;
            }
        }

        if (first_vertex != -1 && second_vertex != -1) //если нашли 2 конца, значит есть ребро
        {
            if (first_vertex == second_vertex)  //признак петли
            {
                g->adj_matrix[first_vertex][second_vertex] += 1; //записываем в поле структуры, в наш массив: 1
                printf("  Edge %d: LOOP at vertex %d\n", j, first_vertex);
            } else //обычное ребро
            {
                g->adj_matrix[first_vertex][second_vertex] += 1; //симметричная запись для матрицы смежности
                g->adj_matrix[second_vertex][first_vertex] += 1;
                printf("  Edge %d: %d -- %d\n", j, first_vertex, second_vertex);
            }
        }
        else if (first_vertex != -1 && !is_loop) //не рыба не мясо
        {
            printf("  WARNING: Edge %d has only one vertex!\n", j);
        }
    } //окончание длинного цикла

    printf("ADJACENCY MATRIX:\n"); //печать матрицы смежности
    printf("     ");
    for (int i = 0; i < vertices; i++) {
        printf("%3d ", i);
    }
    printf("\n     ");
    for (int i = 0; i < vertices; i++) {
        printf("----");
    }
    printf("\n");

    for (int i = 0; i < vertices; i++) {
        printf("%2d | ", i);
        for (int j = 0; j < vertices; j++) {
            printf("%3d ", g->adj_matrix[i][j]);
        }
        printf("\n");
    }

    for (int i = 0; i < vertices; i++) //Освобождаем память временной матрицы инцидентности (строки и массив указателей)
        {
        free(inc_matrix[i]);
    }
    free(inc_matrix);

    printf("Graph successfully created and converted!\n");
    return g; //Сообщаем об успехе и возвращаем указатель на готовый граф
}

void visualizeGraph(Graph* g, const char* filename) //Функция принимает указатель на граф и имя файла (без расширения). Возвращает ничего (void). Создаёт картинку графа через Graphviz
{
    if (g == NULL) {
        printf("ERROR: Graph is NULL\n");
        return;
    }

    char dot_filename[256];
    char png_filename[256];

    snprintf(dot_filename, sizeof(dot_filename), "%s.dot", filename); //безопасная версия printf, которая записывает результат в строку, "%s.dot" к имени файла добавляет расширение .dot
    snprintf(png_filename, sizeof(png_filename), "%s.png", filename); //sizeof(dot_filename) — ограничиваем длину, чтобы не выйти за пределы массива (256 символов)

    FILE* dot_file = fopen(dot_filename, "w"); //создание DOT-файла, создаём файл с именем dot_filename для записи
    if (dot_file == NULL) {
        printf("ERROR: Cannot create file %s\n", dot_filename);
        return;
    }

    fprintf(dot_file, "graph G {\n"); //Записываем в DOT-файл настройки графа:
    fprintf(dot_file, "    layout=neato;\n"); //Использовать алгоритм компоновки neato
    fprintf(dot_file, "    node [shape=circle, style=filled, fillcolor=yellow];\n"); //Вершины — кружочки, залитые желтым
    fprintf(dot_file, "    edge [color=lightgreen];\n\n");

    for (int i = 0; i < g->num_vertices; i++) //Записывает все вершины. Каждая вершина — просто её номер
        {
        fprintf(dot_file, "    %d;\n", i);
    }
    fprintf(dot_file, "\n");

    for (int i = 0; i < g->num_vertices; i++) //Внешний цикл по всем вершинам (строки матрицы)
        {
        for (int j = i; j < g->num_vertices; j++) //Внутренний начинается с i, а не с 0 — чтобы не записывать каждое ребро дважды, граф неориентированный, adj_matrix[i][j] и adj_matrix[j][i] — одно и то же))))
        {
            int edge_count = g->adj_matrix[i][j]; // Берём количество рёбер между вершинами i и j

            if (edge_count > 0) {
                if (i == j) {
                    for (int k = 0; k < edge_count; k++) //Если это петля (i == j): записываем i -- i ; (петля сама в себя)
                        {
                        fprintf(dot_file, "    %d -- %d ;\n", i, i);
                    }
                }
                else //Если обычное ребро: записываем i -- j ;
                {

                        for (int k = 0; k < edge_count; k++) {
                        fprintf(dot_file, "    %d -- %d ;\n", i, j);
                    }
                }
            }
        }
    }

    fprintf(dot_file, "}\n"); //Записывает закрывающую фигурную скобку (конец графа)
    fclose(dot_file);

    printf("DOT file created: %s\n", dot_filename);

    char command[512];
    snprintf(command, sizeof(command), "dot -Tpng \"%s\" -o \"%s\"", dot_filename, png_filename); //Формирует командную строку для вызова dot (программа из Graphviz): dot — программа, -Tpng — выходной формат PNG,
                                                                                                                                        //"graph_original.dot" — входной файл, -o "graph_original.png" — выходной файл

    printf("Generating PNG...\n");
    int result = system(command); //system(command) — выполняет команду через командную строку Windows

    if (result == 0) //Сообщаем, что картинка сохранена, Формируем команду start "graph_original.png" (открыть картинку стандартной программой), Выполняем команду (открывается просмотрщик)
    {
        printf("SUCCESS: Visualization saved to %s\n", png_filename);
        snprintf(command, sizeof(command), "start \"\" \"%s\"", png_filename);
        system(command);
    } else  //Сообщаем, что Graphviz не установлен или не добавлен в PATH, Предлагаем вручную посмотреть DOT-файл
    {
        printf("ERROR: Graphviz command failed.\n");
        printf("Make sure Graphviz is installed and 'dot' is in PATH.\n");
        printf("You can still view the DOT file: %s\n", dot_filename);
    }
}

// ВТОРОЙ ЭТАП

void removeEdgesIncidentToVertex(Graph* g, int vertex) //Функция принимает указатель на граф и номер вершины. Удаляет все рёбра, которые касаются этой вершины
{
            if (g == NULL || vertex < 0 || vertex >= g->num_vertices) //не нулевой казатель? ИЛИ не отрицательный ли номер вершины? ИЛИ не выходит ли номер за границы (вершины от 0 до num_vertices-1)?
    {
        printf("ERROR: Invalid vertex\n");
        return;
    }

    for (int i = 0; i < g->num_vertices; i++) //Проходим по всем вершинам графа (от 0 до num_vertices-1). Для каждой вершины i будем удалять ребро между vertex и i
    {
        if (i == vertex) {
            // Удаляем петли
            g->adj_matrix[vertex][vertex] = 0; //Если дошли до той же самой вершины — значит, смотрим на петлю (ребро из вершины в саму себя)
                                            //g->adj_matrix[vertex][vertex] = 0 — обнуляем значение на пересечении строки vertex и столбца vertex
        } else {
            // Удаляем рёбра между vertex и i
            g->adj_matrix[vertex][i] = 0; //обнуляем ребро из vertex в i
            g->adj_matrix[i][vertex] = 0; //обнуляем ребро из i в vertex
            //Граф неориентированный, матрица смежности симметричная. Чтобы удалить ребро, нужно обнулить оба симметричных элемента
        }
    }
    printf("Removed all edges incident to vertex %d\n", vertex);
}

void computeDegrees(Graph* g, int* degrees) //Функция принимает указатель на граф и указатель на массив degrees (куда запишем результаты). Вычисляет степень каждой вершины и сохраняет в этом массиве
{
    for (int i = 0; i < g->num_vertices; i++)  //Проходим по всем вершинам графа от 0 до num_vertices-1. Для каждой вершины i будем считать её степень.
    {
        degrees[i] = 0; //Обнуление текущей степени
        for (int j = 0; j < g->num_vertices; j++)  // Для каждой пары (i, j) смотрим, сколько там рёбер.
        {
            if (i == j) {
                // Петля даёт 2 к степени
                degrees[i] += g->adj_matrix[i][j] * 2;
            } else {
                degrees[i] += g->adj_matrix[i][j];
            }
        }
    }
}

int compareVerticesByDegree(const void* a, const void* b, void* degrees) //Функция-компаратор для сравнения двух вершин по их степени.
{   //a и b — это указатели типа const void* (универсальные указатели)
    int vertex_a = *(int*)a; //(int*)a преобразуем в указатель на int, *(int*)a — берём значение (номер вершины)
    int vertex_b = *(int*)b;
    int* deg = (int*)degrees; //Преобразуем указатель на массив степеней из void* в int*

    if (deg[vertex_a] < deg[vertex_b]) return -1; //Если степень вершины a меньше — возвращаем -1 (значит a должна быть раньше)
    if (deg[vertex_a] > deg[vertex_b]) return 1; //Если степень вершины a больше — возвращаем 1 (значит b раньше)
    return vertex_a - vertex_b; // при равных степенях сортируем по номеру вершины
}

void sortVerticesByDegree(int* vertices, int* degrees, int n) //Функция сортирует массив vertices по степеням вершин (по возрастанию)
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (degrees[vertices[j]] > degrees[vertices[j + 1]]) {
                int temp = vertices[j];
                vertices[j] = vertices[j + 1];
                vertices[j + 1] = temp;
            }
            else if (degrees[vertices[j]] == degrees[vertices[j + 1]] && vertices[j] > vertices[j + 1]) {
                int temp = vertices[j];
                vertices[j] = vertices[j + 1];
                vertices[j + 1] = temp;
            }
        }
    }
}

void printSortedVertices(Graph* g, int* degrees)// Функция создаёт массив вершин, сортирует их по степени, выводит результат
{
    int* sorted_vertices = (int*)malloc(g->num_vertices * sizeof(int)); //Выделяет память под массив для отсортированных вершин. Если не вышло — ошибка и выход.
    if (sorted_vertices == NULL) {
        printf("ERROR: Cannot allocate memory for sorting\n");
        return;
    }

    for (int i = 0; i < g->num_vertices; i++) { //Заполняет массив числами от 0 до num_vertices-1. Изначально вершины идут по порядку.
        sorted_vertices[i] = i;
    }

    sortVerticesByDegree(sorted_vertices, degrees, g->num_vertices); //Сортирует массив по степени (вызов предыдущей функции).

    printf("Vertices sorted by degree (ascending):\n");
    for (int i = 0; i < g->num_vertices; i++) {
        printf("  Vertex %d: degree %d\n", sorted_vertices[i], degrees[sorted_vertices[i]]); //Выводит результат — вершины в порядке возрастания степени и их степени
    }

    free(sorted_vertices); //очищаем память
}

int main() {
    SetConsoleOutputCP(866); //Устанавливает кодовую страницу консоли на 866 (кириллица

    printf("Variant 17: incidence matrix -> adjacency matrix\n");

    Graph* g = readGraphFromFile("grath3.txt"); //Функция открывает файл, читает матрицу инцидентности, преобразует в матрицу смежности

    if (g == NULL) {
        printf("ERROR: Failed to create graph.\n");
        printf("Press Enter to exit...");
        getchar();
        return 1;
    }

    printf("SUCCESS: Graph created and converted!\n");

    printf("VISUALIZATION of original graph:\n");
    visualizeGraph(g, "graph_original"); //Функция создаёт graph_original.dot и graph_original.png

    // ВТОРОЙ ЭТАП
    printf("\n========== SECOND STAGE ==========\n");

    int target_vertex;
    printf("Enter vertex number to remove its incident edges: ");
    scanf("%d", &target_vertex); //читаем число с клавиатуры и сохраняем в target_vertex

    if (target_vertex < 0 || target_vertex >= g->num_vertices) {
        printf("ERROR: Vertex %d does not exist (0..%d)\n", target_vertex, g->num_vertices - 1);
    } else {

        removeEdgesIncidentToVertex(g, target_vertex);// Удаляем рёбра, смежные с заданной вершиной


        printf("\nADJACENCY MATRIX AFTER REMOVAL:\n");// Выводим матрицу смежности после удаления
        printf("     ");
        for (int i = 0; i < g->num_vertices; i++) {
            printf("%3d ", i);
        }
        printf("\n     ");
        for (int i = 0; i < g->num_vertices; i++) {
            printf("----");
        }
        printf("\n");

        for (int i = 0; i < g->num_vertices; i++) {
            printf("%2d | ", i);
            for (int j = 0; j < g->num_vertices; j++) {
                printf("%3d ", g->adj_matrix[i][j]);
            }
            printf("\n");
        }


        int* degrees = (int*)malloc(g->num_vertices * sizeof(int));
        if (degrees != NULL) {
            computeDegrees(g, degrees); //Если память выделилась — вызываем computeDegrees, которая заполняет массив

            printf("\nDEGREES OF VERTICES AFTER REMOVAL:\n");
            for (int i = 0; i < g->num_vertices; i++) {
                printf("  Vertex %d: degree %d\n", i, degrees[i]);
            }

            printSortedVertices(g, degrees);// Сортируем вершины по степени


            free(degrees);
        } else {
            printf("ERROR: Cannot allocate memory for degrees\n");
        }


        printf("\nVISUALIZATION of modified graph:\n");

        visualizeGraph(g, "graph_modified");// Визуализируем модифицированный граф
    }

    printf("\nFreeing graph memory...\n"); //Очистка памяти
    for (int i = 0; i < g->num_vertices; i++) {
        free(g->adj_matrix[i]);
    }
    free(g->adj_matrix);
    free(g);
    printf("Memory freed successfully.\n");

    printf("Press Enter to exit...");
    getchar();
    getchar();

    return 0;
}
