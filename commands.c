#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "createfield.h"

// Реализация команды MOVE

// Проверка на указатели на динозавра и поле не NULL
int move_dino(struct Dino *dino, struct Field *field, Direction direction) {
    if (dino == NULL || field == NULL) {
        printf("Не задано поле или динозавр");
        return 0;
    }
    
    // Сохраняем текущую позицию
    int old_x = dino->x;
    int old_y = dino->y;
    //Сохраняет текущие координаты динозавра и инициализирует новые координаты теми же значениями. Чтобы потом восстановить клетку, на которой стоял динозавр, и вычислить новую позицию.
    int new_x = old_x;
    int new_y = old_y;
    
    // Вычисляем новые координаты в зависимости от направления (с учетом топологии тора)
    switch (direction) {
        case DIR_UP:
            new_y = (old_y - 1 + field->height) % field->height; // вычитаем 1 (вверх), а потом прибавляем высоту чтобы избежать отрицательных значений; делим на высоту поля
            break;
        case DIR_DOWN:
            new_y = (old_y + 1) % field->height; // прибавляем 1 (вниз), делим на высоту поля
            break;
        case DIR_LEFT:
            new_x = (old_x - 1 + field->width) % field->width; // вычитем 1 (влево), прибаляем высоту чтобы избежать отрицательных значений; делим на ширину поля
            break;
        case DIR_RIGHT:
            new_x = (old_x + 1) % field->width; // прибавляем 1 (вправо), делим на ширину поля
            break;
        default: // Если case не равен 0,1,2,3
            printf("ERROR: Unknown direction\n");
            return 0;
    }
    
    // Проверяем, можно ли переместиться в новую клетку
    char target_cell = field->tiles[new_y][new_x];
    
    // Проверяем препятствия
    if (target_cell == '%') {  // Яма - возвращает ошибку, завершает программу
        printf("ERROR: Cannot move into a hole at (%d,%d)\n", new_x, new_y);
        return 0;
    } else if (target_cell == '^' || target_cell == '&' || target_cell == '@') {
        // Гора, дерево или камень - возвращает предупреждение
        printf("WARNING: Cannot move into obstacle '%c' at (%d,%d), command ignored\n", 
               target_cell, new_x, new_y);
        return 1;  // Игнорируем команду, но не завершаем программу
    }
    
    // Восстанавливаем старую клетку
    field->tiles[old_y][old_x] = dino->steppedCell;
    
    // Сохраняем символ новой клетки и перемещаем динозавра
    dino->steppedCell = field->tiles[new_y][new_x];
    field->tiles[new_y][new_x] = '#';
    
    // Обновляем координаты динозавра
    dino->x = new_x;
    dino->y = new_y;
    
    return 1;
}

// Реализация команды PRNT

int paint_cell(struct Dino *dino, struct Field *field, char color) {
    if (dino == NULL || field == NULL) {
        printf("ERROR: Dino or field not initialized\n");
        return 0;
    }
    
    // Проверяем, что цвет - строчная латинская буква
    if (color < 'a' || color > 'z') {
        printf("ERROR: Invalid paint color '%c'. Must be a lowercase letter (a-z)\n", color);
        return 0;
    }
    
    // Проверяем, что динозавр находится в допустимых пределах поля
    if (dino->x < 0 || dino->x >= field->width || dino->y < 0 || dino->y >= field->height) {
        printf("ERROR: Dino is outside field boundaries\n");
        return 0;
    }
    
    // Окрашиваем клетку, на которой стоит динозавр
    // Обновляем steppedCell, чтобы при уходе динозавра клетка оставалась окрашенной
    dino->steppedCell = color;
    field->tiles[dino->y][dino->x] = color;
    
    printf("Cell at (%d,%d) painted with color '%c'\n", dino->x, dino->y, color);
    return 1;
}


// Вспомогательная функция для преобразования строки (UP, DOWN, LEFT, RIGHT) в направление\тип Direction (DIR_UP, ...)
Direction parse_direction(const char *dir_str) {
    if (strcmp(dir_str, "UP") == 0) return DIR_UP;
    if (strcmp(dir_str, "DOWN") == 0) return DIR_DOWN;
    if (strcmp(dir_str, "LEFT") == 0) return DIR_LEFT;
    if (strcmp(dir_str, "RIGHT") == 0) return DIR_RIGHT;
    return -1; // Неизвестное направление
} 

// Вспомогательная функция для обработки команды PAINT из строки
int handle_paint_command(struct Dino *dino, struct Field *field, const char *color_str) {
    if (color_str == NULL || strlen(color_str) != 1) {
        printf("ERROR: Invalid paint command format. Use: PAINT <letter>\n");
        return 0;
    }
    
    char color = color_str[0];
    return paint_cell(dino, field, color);
}

int create_obstacle(struct Dino *dino, struct Field *field, Direction direction, char obstacle_type) {
    if (dino == NULL || field == NULL) {
        printf("ERROR: Dino or field not initialized\n");
        return 0;
    }

    // Вычисляем координаты целевой клетки
    int target_x = dino->x;
    int target_y = dino->y;
    
    switch (direction) {
        case DIR_UP:
            target_y = (dino->y - 1 + field->height) % field->height;
            break;
        case DIR_DOWN:
            target_y = (dino->y + 1) % field->height;
            break;
        case DIR_LEFT:
            target_x = (dino->x - 1 + field->width) % field->width;
            break;
        case DIR_RIGHT:
            target_x = (dino->x + 1) % field->width;
            break;
        default:
            printf("ERROR: Unknown direction\n");
            return 0;
    }
    
    // Проверяем, что целевая клетка не занята динозавром
    if (target_x == dino->x && target_y == dino->y) {
        printf("ERROR: Cannot create obstacle on dino's position\n");
        return 0;
    }
    
    char current_cell = field->tiles[target_y][target_x];
    
    // Обрабатываем специальные случаи взаимодействия
    if (obstacle_type == '^' && current_cell == '%') {
        // Насыпание горы в яму - засыпаем яму
        field->tiles[target_y][target_x] = '_';
        printf("Hole filled with mound at (%d,%d), cell is now empty\n", target_x, target_y);
    } else if (obstacle_type == '%' && current_cell == '^') {
        // Копание ямы в горе - убираем гору
        field->tiles[target_y][target_x] = '%';
        printf("Mound dug out at (%d,%d), cell is now a hole\n", target_x, target_y);
    } else if (current_cell == '_' || (current_cell >= 'a' && current_cell <= 'z')) {
        // Создаем препятствие на пустой или окрашенной клетке
        field->tiles[target_y][target_x] = obstacle_type;
        printf("%s created at (%d,%d)\n", 
               (obstacle_type == '%') ? "Hole" : "Mound", target_x, target_y);
    } else {
        // Нельзя создать препятствие на другом препятствии (кроме специальных случаев выше)
        printf("WARNING: Cannot create %s on existing obstacle '%c' at (%d,%d)\n",
               (obstacle_type == '%') ? "hole" : "mound", current_cell, target_x, target_y);
        return 1; // Игнорируем команду, но не завершаем программу
    }
    
    return 1;
}

// Функция для обработки команды DIG
int handle_dig_command(struct Dino *dino, struct Field *field, const char *direction_str) {
    if (dino == NULL || field == NULL) {
        printf("ERROR: Dino or field not initialized\n");
        return 0;
    }
    
    Direction dir = parse_direction(direction_str);
    if (dir == -1) {
        printf("ERROR: Unknown direction '%s'\n", direction_str);
        return 0;
    }
    
    return create_obstacle(dino, field, dir, '%');
}

// Функция для обработки команды MOUND
int handle_mound_command(struct Dino *dino, struct Field *field, const char *direction_str) {
    if (dino == NULL || field == NULL) {
        printf("ERROR: Dino or field not initialized\n");
        return 0;
    }
    
    Direction dir = parse_direction(direction_str);
    if (dir == -1) {
        printf("ERROR: Unknown direction '%s'\n", direction_str);
        return 0;
    }
    
    return create_obstacle(dino, field, dir, '^');
}

// Реализация команды JUMP
int jump_dino(struct Dino *dino, struct Field *field, Direction direction, int n) {
    // Проверка на нулевые указатели динозавра и поля
    if (dino == NULL || field == NULL) {
        printf("ERROR: Cannot perform jump - field or dinosaur is not initialized\n");
        return 0;  // Возвращаем 0 при ошибке
    }
    
    // Проверка что длина прыжка положительна
    if (n <= 0) {
        printf("ERROR: Jump length must be positive\n");
        return 0;  // Возвращаем 0 при недопустимой длине прыжка
    }

    // Инициализация переменных для изменения координат
    int dx = 0, dy = 0;
    
    // Определение направления движения по осям X и Y
    switch (direction) {
        case DIR_UP:    dy = -1; break;    // Вверх - уменьшение Y
        case DIR_DOWN:  dy = 1; break;     // Вниз - увеличение Y
        case DIR_LEFT:  dx = -1; break;    // Влево - уменьшение X
        case DIR_RIGHT: dx = 1; break;     // Вправо - увеличение X
        default: 
            printf("ERROR: Unknown direction\n");
            return 0;  // Возвращаем 0 при неизвестном направлении
    }

    // Сохраняем текущие координаты динозавра
    int old_x = dino->x;
    int old_y = dino->y;
    
    // Изначально планируем прыгнуть на n клеток
    int target_step = n;

    // Проверяем путь на наличие гор - проходим по всем клеткам на пути прыжка
    for (int i = 1; i <= n; i++) {
        // Вычисляем координаты текущей проверяемой клетки с учетом тороидальной топологии
        int new_x = (old_x + i * dx + field->width) % field->width;
        int new_y = (old_y + i * dy + field->height) % field->height;

        // Если встречаем гору, уменьшаем длину прыжка до предыдущей клетки
        if (field->tiles[new_y][new_x] == '^') {
            target_step = i - 1;  // Останавливаемся перед горой
            printf("WARNING: Stopped due to mountain at (%d,%d), jumped only %d steps\n", 
                   new_x, new_y, target_step);
            break;  // Прерываем цикл проверки
        }
    }

    // Вычисляем конечные координаты приземления с учетом тороидальной топологии
    int land_x = (old_x + target_step * dx + field->width) % field->width;
    int land_y = (old_y + target_step * dy + field->height) % field->height;

    // Проверяем, что клетка приземления не является ямой (только если прыжок состоялся)
    if (target_step >= 1 && field->tiles[land_y][land_x] == '%') {
        printf("ERROR: Cannot land into a hole at (%d,%d)\n", land_x, land_y);
        return 0;  // Возвращаем 0 при попытке приземлиться в яму
    }

    // Если прыжок не состоялся (target_step = 0), остаемся на месте
    if (target_step == 0) {
        return 1;  // Возвращаем 1 - команда обработана (хоть и без перемещения)
    }

    // Восстанавливаем символ на старой позиции динозавра
    field->tiles[old_y][old_x] = dino->steppedCell;

    // Сохраняем символ новой клетки и размещаем динозавра на новой позиции
    dino->steppedCell = field->tiles[land_y][land_x];
    field->tiles[land_y][land_x] = '#';  // '#' - символ динозавра

    // Обновляем координаты динозавра в структуре
    dino->x = land_x;
    dino->y = land_y;

    return 1;  // Успешное выполнение команды
}

// Вспомогательная функция для получения координат целевой клетки
void get_target_cell(struct Dino *dino, struct Field *field, Direction direction, int *target_x, int *target_y) {
    *target_x = dino->x;
    *target_y = dino->y;
    
    switch (direction) {
        case DIR_UP:
            *target_y = (dino->y - 1 + field->height) % field->height;
            break;
        case DIR_DOWN:
            *target_y = (dino->y + 1) % field->height;
            break;
        case DIR_LEFT:
            *target_x = (dino->x - 1 + field->width) % field->width;
            break;
        case DIR_RIGHT:
            *target_x = (dino->x + 1) % field->width;
            break;
    }
}

// Вспомогательная функция для получения координат из произвольной точки
void get_target_cell_from(int start_x, int start_y, struct Field *field, Direction direction, int *target_x, int *target_y) {
    *target_x = start_x;
    *target_y = start_y;
    
    switch (direction) {
        case DIR_UP:
            *target_y = (start_y - 1 + field->height) % field->height;
            break;
        case DIR_DOWN:
            *target_y = (start_y + 1) % field->height;
            break;
        case DIR_LEFT:
            *target_x = (start_x - 1 + field->width) % field->width;
            break;
        case DIR_RIGHT:
            *target_x = (start_x + 1) % field->width;
            break;
    }
}

// Функция создания дерева
int create_tree(struct Dino *dino, struct Field *field, Direction direction) {
    // Проверка валидности указателей
    if (dino == NULL || field == NULL) {
        printf("ERROR: Dino or field not initialized\n");
        return 0;
    }

    // Получаем координаты целевой клетки куда хотим поставить дерево
    int target_x, target_y;
    get_target_cell(dino, field, direction, &target_x, &target_y);
    
    // Проверяем, что клетка подходит для дерева (т.е. пустая или окрашенная)
    char current_cell = field->tiles[target_y][target_x];
    if (current_cell != '_' && !(current_cell >= 'a' && current_cell <= 'z')) {
        printf("WARNING: Cannot grow tree on cell '%c' at (%d,%d)\n", current_cell, target_x, target_y);
        return 1; // Игнорируем команду
    }
    // Создаем дерево
    field->tiles[target_y][target_x] = '&';
    printf("Tree grown at (%d,%d)\n", target_x, target_y);
    return 1;
}

// Функция срубания дерева
int cut_tree(struct Dino *dino, struct Field *field, Direction direction) {
    // Проверка валидности указателей
    if (dino == NULL || field == NULL) {
        printf("ERROR: Dino or field not initialized\n");
        return 0;
    }

    // Получаем координаты целевой клетки где хотим срубить дерево
    int target_x, target_y;
    get_target_cell(dino, field, direction, &target_x, &target_y);
    
    // Проверяем, что в клетке есть дерево
    if (field->tiles[target_y][target_x] != '&') {
        printf("WARNING: No tree to cut at (%d,%d)\n", target_x, target_y);
        return 1; // Игнорируем команду
    }

    // Удаляем дерево, восстанавливая пустую клетку
    field->tiles[target_y][target_x] = '_';
    printf("Tree cut at (%d,%d)\n", target_x, target_y);
    return 1;
}

// Обработчик команды GROW
int handle_grow_command(struct Dino *dino, struct Field *field, const char *direction_str) {
    // Парсим направление из строки
    Direction dir = parse_direction(direction_str);
    if (dir == -1) {
        printf("ERROR: Unknown direction '%s'\n", direction_str);
        return 0;
    }
    // Создаем дерево
    return create_tree(dino, field, dir);
}

// Обработчик команды CUT
int handle_cut_command(struct Dino *dino, struct Field *field, const char *direction_str) {
    // Парсим направление из строки
    Direction dir = parse_direction(direction_str);
    if (dir == -1) {
        printf("ERROR: Unknown direction '%s'\n", direction_str);
        return 0;
    }
    // Срубаем дерево
    return cut_tree(dino, field, dir);
}

// Функция создания камня
int create_stone(struct Dino *dino, struct Field *field, Direction direction) {
    // Проверка валидности указателей
    if (dino == NULL || field == NULL) {
        printf("ERROR: Dino or field not initialized\n");
        return 0;
    }

    // Получаем координаты целевой клетки
    int target_x, target_y;
    get_target_cell(dino, field, direction, &target_x, &target_y);
    
    // Проверяем, что клетка пустая
    if (field->tiles[target_y][target_x] != '_') {
        printf("WARNING: Cannot make stone on non-empty cell at (%d,%d)\n", target_x, target_y);
        return 1; // Игнорируем команду
    }

    // Создаем камень
    field->tiles[target_y][target_x] = '@';
    printf("Stone created at (%d,%d)\n", target_x, target_y);
    return 1;
}

// Функция перемещения камня
int push_stone(struct Dino *dino, struct Field *field, Direction direction) {
    // Проверка валидности указателей
    if (dino == NULL || field == NULL) {
        printf("ERROR: Dino or field not initialized\n");
        return 0;
    }

    // Получаем координаты камня (соседняя клетка)
    int stone_x, stone_y;
    get_target_cell(dino, field, direction, &stone_x, &stone_y);
    
    // Проверяем, что в клетке есть камень
    if (field->tiles[stone_y][stone_x] != '@') {
        printf("WARNING: No stone to push at (%d,%d)\n", stone_x, stone_y);
        return 1; // Игнорируем команду
    }

    // Вычисляем координаты, куда будем двигать камень
    int new_x, new_y;
    get_target_cell_from(stone_x, stone_y, field, direction, &new_x, &new_y);
    
    // Получаем символ целевой клетки для камня
    char target_cell = field->tiles[new_y][new_x];
    
    // Проверяем препятствия для перемещения камня
    if (target_cell == '^' || target_cell == '&' || target_cell == '@') {
        printf("WARNING: Cannot push stone into obstacle at (%d,%d)\n", new_x, new_y);
        return 1; // Игнорируем команду
    }
    
    // Обрабатываем специальные случаи перемещения камня
    if (target_cell == '%') {
        // Камень падает в яму - яма засыпается
        field->tiles[new_y][new_x] = '_';
        field->tiles[stone_y][stone_x] = '_';
        printf("Stone pushed into hole at (%d,%d), hole filled\n", new_x, new_y);
    } else if (target_cell == '_') {
        // Камень перемещается на пустую клетку
        field->tiles[new_y][new_x] = '@';
        field->tiles[stone_y][stone_x] = '_';
        printf("Stone pushed to (%d,%d)\n", new_x, new_y);
    } else {
        // Нельзя переместить камень на занятую клетку
        printf("WARNING: Cannot push stone into cell with symbol '%c' at (%d,%d)\n", target_cell, new_x, new_y);
        return 1; // Игнорируем команду
    }
    
    return 1;
}

// Обработчик команды MAKE
int handle_make_command(struct Dino *dino, struct Field *field, const char *direction_str) {
    // Парсим направление из строки
    Direction dir = parse_direction(direction_str);
    if (dir == -1) {
        printf("ERROR: Unknown direction '%s'\n", direction_str);
        return 0;
    }
    // Создаем камень
    return create_stone(dino, field, dir);
}

// Обработчик команды PUSH
int handle_push_command(struct Dino *dino, struct Field *field, const char *direction_str) {
    // Парсим направление из строки
    Direction dir = parse_direction(direction_str);
    if (dir == -1) {
        printf("ERROR: Unknown direction '%s'\n", direction_str);
        return 0;
    }
    // Перемещаем камень
    return push_stone(dino, field, dir);
}