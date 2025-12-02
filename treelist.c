#include "huffman.h"
#include <time.h>

// Функции для работы с деревом и списком

// Создание нового узла дерева
Node* create_node(unsigned char symbol, unsigned int freq) {
    Node *node = (Node*)malloc(sizeof(Node)); // выделяет блок памяти размером со структуру Node
    if (node) {
        node->symbol = symbol; // символ
        node->freq = freq; // частота символа
        node->left = NULL; // левый потомок
        node->right = NULL; // правый потомок
    }
    return node;
}

// Создание нового элемента списка c указателем на следующий элемент
ListNode* create_list_node(Node *tree_node) {
    ListNode *list_node = (ListNode*)malloc(sizeof(ListNode)); //выделяет память для структуры ListNode
    if (list_node) {
        list_node->tree_node = tree_node;
        list_node->next = NULL;
    }
    return list_node;
}

// Вставка в упорядоченный список по возрастанию частоты
void insert_sorted(ListNode **head, ListNode *new_node) {
    if (*head == NULL || new_node->tree_node->freq < (*head)->tree_node->freq) { // Проверка на вставку в начало списка
        new_node->next = *head;
        *head = new_node;
    } else {
        ListNode *current = *head; // Поиск места для вставки
        while (current->next != NULL && 
               current->next->tree_node->freq <= new_node->tree_node->freq) {
            current = current->next;
        }
        new_node->next = current->next; // Сама вставка
        current->next = new_node;
    }
}

// Построение дерева Хаффмана с использованием упорядоченного списка
Node* build_huffman_tree(unsigned int *freq_table) {
    ListNode *head = NULL; // Указатель на голову упорядоченного списка
    int unique_chars = 0;
    
    // Создаем список листьев
    for (int i = 0; i < 256; i++) {
        if (freq_table[i] > 0) {
            Node *leaf = create_node((unsigned char)i, freq_table[i]);
            ListNode *list_node = create_list_node(leaf);
            insert_sorted(&head, list_node);
            unique_chars++;
        }
    }
    
    // Обработка особых случаев
    if (unique_chars == 0) {
        return NULL;  // Файл пустой
    }
    
    // Обработка случая с одним уникальным символом
    if (unique_chars == 1) {
        // Для одного символа создаем искусственное дерево
        Node *single_node = head->tree_node;
        
        // Создаем корень с левым потомком = сам символ, правый = NULL
        Node *root = create_node(0, single_node->freq);
        root->left = single_node;
        root->right = NULL;
        
        free(head);  // Освобождаем только ListNode, Node остается в дереве
        return root;
    }
    
    // Безопасное построение дерева для 2+ символов
    while (head != NULL && head->next != NULL) {
        // Извлекаем два узла с наименьшими частотами
        ListNode *first = head;
        ListNode *second = head->next;
        
        // Обновление головы списка
        head = second->next;  // Может стать NULL
        
        // Создаем объединенный узел
        Node *combined = create_node(0, first->tree_node->freq + second->tree_node->freq);
        combined->left = first->tree_node;
        combined->right = second->tree_node;
        
        // Вставляем объединенный узел обратно в список
        ListNode *new_list_node = create_list_node(combined);
        insert_sorted(&head, new_list_node);
        
        // Освобождаем ТОЛЬКО ListNode, а не Node
        // Node остаются в дереве и будут освобождены в free_tree()
        free(first);
        free(second);
    }
    
    // В списке остался один элемент - корень дерева
    Node *root = head->tree_node;
    free(head);  // Освобождаем последний ListNode
    return root;
}

// Освобождение дерева Хаффмана 
void free_tree(Node *root) {
    if (root == NULL) return;
    
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}


// Функции для работы с кодами


// Рекурсивная генерация кодов Хаффмана
void generate_codes(Node *root, char *code, int depth, CodeTable *code_table) {
    if (root == NULL) return;
    
    // Если это лист
    if (root->left == NULL && root->right == NULL) {
        code_table[root->symbol].symbol = root->symbol; // Cохраняем символ
        code_table[root->symbol].code = (char*)malloc(depth + 1); // Выделяем память для строки кода (+1 для нуль-терминатора)
        if (code_table[root->symbol].code) {
            strncpy(code_table[root->symbol].code, code, depth); // Копируем код из буфера
            code_table[root->symbol].code[depth] = '\0'; // Добавляем нуль-терминатор
            code_table[root->symbol].code_length = depth; // Сохраняем длину кода
        }
        return;
    }
    
    // Левое поддерево - добавляем '0'
    if (depth < 256) { // Защита от переполнения
        code[depth] = '0';
        generate_codes(root->left, code, depth + 1, code_table);
    }
    
    // Правое поддерево - добавляем '1'
    if (depth < 256) {
        code[depth] = '1';
        generate_codes(root->right, code, depth + 1, code_table);
    }
}

// Освобождение таблицы кодов
void free_code_table(CodeTable *code_table, int size) {
    for (int i = 0; i < size; i++) { // Проходим по всем элементам списка 
        if (code_table[i].code != NULL) { // Освобаждаем память, если у символа нет кода
            free(code_table[i].code);
        }
    }
}


// Функции для ввода и вывода 

// Построение таблицы частот
void build_frequency_table(FILE *file, unsigned int *freq_table) {
    memset(freq_table, 0, 256 * sizeof(unsigned int)); // memset заполняет память нулями; по 4 байта на символ; все частоты изначально 0
    
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        freq_table[(unsigned char)ch]++;
    }
    
    rewind(file);
}

// Запись бита в буфер
void write_bit(FILE *file, unsigned char bit, unsigned char *bit_buffer, int *bit_count) {
    *bit_buffer = (*bit_buffer << 1) | (bit & 1);
    (*bit_count)++;
    
    if (*bit_count == 8) {
        fputc(*bit_buffer, file);
        *bit_buffer = 0;
        *bit_count = 0;
    }
}

// Запись оставшихся битов
void flush_bits(FILE *file, unsigned char *bit_buffer, int *bit_count) {
    if (*bit_count > 0) {
        *bit_buffer = *bit_buffer << (8 - *bit_count);
        fputc(*bit_buffer, file);
        *bit_buffer = 0;
        *bit_count = 0;
    }
}

// Чтение бита из файла
unsigned char read_bit(FILE *file, unsigned char *bit_buffer, int *bit_count, int *eof) {
    if (*bit_count == 0) {
        int byte = fgetc(file);
        if (byte == EOF) {
            *eof = 1;
            return 0;
        }
        *bit_buffer = (unsigned char)byte;
        *bit_count = 8;
    }
    
    unsigned char bit = (*bit_buffer >> 7) & 1;
    *bit_buffer = *bit_buffer << 1;
    (*bit_count)--;
    
    return bit;
}

// Функции для кодирования и декодирования

// Кодирование файла
void encode_file(const char *input_file, const char *output_file) {
    clock_t start = clock();
    FILE *in = fopen(input_file, "rb");
    if (!in) {
        printf("ERROR: the input file could not be opened %s\n", input_file);
        return;
    }
    
    FILE *out = fopen(output_file, "wb");
    if (!out) {
        printf("ERROR: Failed to create the output file%s\n", output_file);
        fclose(in);
        return;
    }
    
    printf("Encoding the file %s...\n", input_file);
    
    // Шаг 1: Построение таблицы частот
    unsigned int freq_table[256];
    build_frequency_table(in, freq_table);
    
    // Шаг 2: Построение дерева Хаффмана
    Node *root = build_huffman_tree(freq_table);
    if (root == NULL) {
        printf("Error: The input file is empty\n");
        fclose(in);
        fclose(out);
        return;
    }
    
    // Шаг 3: Генерация кодов
    CodeTable code_table[256];
    memset(code_table, 0, sizeof(code_table));
    
    char code_buffer[256];
    generate_codes(root, code_buffer, 0, code_table);
    
    // Шаг 4: Запись таблицы частот в выходной файл
    fwrite(freq_table, sizeof(unsigned int), 256, out);
    
    // Шаг 5: Кодирование данных
    unsigned char bit_buffer = 0;
    int bit_count = 0;
    
    int ch;
    while ((ch = fgetc(in)) != EOF) {
        char *code = code_table[(unsigned char)ch].code;
        if (code) {
            for (int i = 0; code[i] != '\0'; i++) {
                write_bit(out, code[i] == '1' ? 1 : 0, &bit_buffer, &bit_count);
            }
        }
    }
    
    flush_bits(out, &bit_buffer, &bit_count);
    
    // Статистика
    fseek(in, 0, SEEK_END);
    long original_size = ftell(in);
    fseek(out, 0, SEEK_END);
    long compressed_size = ftell(out);
    
     clock_t end = clock();
    double elapsed_seconds = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("The original size: %ld bytes\n", original_size);
    printf("Compressed size: %ld bytes\n", compressed_size);
    printf("Compression ratio: %.2f\n", 
           (double)compressed_size / original_size);
    
    // Освобождение ресурсов
    free_tree(root);    free_code_table(code_table, 256);
    fclose(in);
    fclose(out);

    printf("Encoding time: %.6f seconds\n", elapsed_seconds);
    printf("The encoding is complete. The result is in %s\n", output_file);
}

// Декодирование файла
void decode_file(const char *input_file, const char *output_file) {
    clock_t start = clock();
    FILE *in = fopen(input_file, "rb");
    if (!in) {
        printf("ERROR: Failed to open input file %s\n", input_file);
        return;
    }
    
    FILE *out = fopen(output_file, "wb");
    if (!out) {
        printf("ERROR: Failed to create output file %s\n", output_file);
        fclose(in);
        return;
    }
    
    printf("Decoding file %s...\n", input_file);
    
    // Чтение таблицы частот
    unsigned int freq_table[256];
    if (fread(freq_table, sizeof(unsigned int), 256, in) != 256) {
        printf("ERROR: Invalid compressed file format\n");
        fclose(in);
        fclose(out);
        return;
    }
    
    // Построение дерева Хаффмана
    Node *root = build_huffman_tree(freq_table);
    if (root == NULL) {
        printf("ERROR: Failed to build Huffman tree\n");
        fclose(in);
        fclose(out);
        return;
    }
    
    // Декодирование данных
    unsigned char bit_buffer = 0;
    int bit_count = 0;
    int eof = 0; // Флаг, созданный для цикла
    
    Node *current = root;

    // Подсчитаем общее количество символов для проверки
    int total_symbols = 0;
    for (int i = 0; i < 256; i++) {
        total_symbols += freq_table[i];
    }
    
    int decoded_symbols = 0;
    
    while (!eof) {
        unsigned char bit = read_bit(in, &bit_buffer, &bit_count, &eof);
        
        if (!eof) {
            if (bit == 0) {
                current = current->left;
            } else {
                current = current->right;
            }
            
            // Если достигли листа
            if (current->left == NULL && current->right == NULL) {
                fputc(current->symbol, out);
                current = root;
                decoded_symbols++;

                // Дополнительная проверка: если декодировали все символы, выходим
                if (decoded_symbols >= total_symbols) break;
            }
        }
    }
    clock_t end = clock();
    double elapsed_seconds = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Decoding time: %.6f seconds\n", elapsed_seconds);
    printf("Decoding completed. Result in %s\n", output_file);
    
    // Освобождение ресурсов
    free_tree(root);
    fclose(in);
    fclose(out);
}