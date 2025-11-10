// Структура поля (прямоугольника), где width - ширина прямоугольника, height - высота прямоугольника
struct Field {
    char **tiles;
    int width, height;
};

// Сруктура нашего динозавра. x, y - координаты динозавра по горизонтали и вертикали соответственно.
struct Dino {
    int x, y;
    char steppedCell;  // Символ, который был на клетке до прихода динозавра;
};

// Набор целочисленных констант типа Direction (направление движения) чтобы потом писать было удобнее
typedef enum {
    DIR_UP, // = 0
    DIR_DOWN, // = 1
    DIR_LEFT, // = 2
    DIR_RIGHT // = 3
} Direction;

// Функция для инициализации динозавра
int init_dino(struct Dino*, struct Field *field, int, int);

// Функция для создания поля
int init_field(struct Field*, int, int);

// Функция для определения размеров поля, а затем для его инициализации (чтение первой строки)
int parse_line_init_field(char *const, struct Field*);

// Функция для определения позиции динозавра (чтение второй строки)
int parse_line_init_dino(char *const, struct Dino*, struct Field*);

// Функция для того, чтобы печатать поле в терминале
void print_field(struct Field*);

// Функция для освобождения памяти поля
void free_field(struct Field *);

// Функция для сохранения поля в файл
void print_field_in_file(struct Field* field, const char* filename);

// Функция перемещения
int move_dino(struct Dino *dino, struct Field *field, Direction direction);

// Объявления функций для работы с командами

// Функция для покраски клеток
int paint_cell(struct Dino *dino, struct Field *field, char color);

// Функция для обработки команды PAINT
int handle_paint_command(struct Dino *dino, struct Field *field, const char *color_str);

// Преобразует строку ("UP", "DOWN", etc.) в enum Direction
Direction parse_direction(const char *dir_str);

// функция для обработки команды DIG
int handle_dig_command(struct Dino *dino, struct Field *field, const char *direction_str);

// Функция для обработки команды MOUND
int handle_mound_command(struct Dino *dino, struct Field *field, const char *direction_str);

// Функция для прыжка
int jump_dino(struct Dino *dino, struct Field *field, Direction direction, int distance);

// Функция для создания препятствий
int create_obstacle(struct Dino *dino, struct Field *field, Direction direction, char obstacle_type);

// Команды камней  

// Функция создания камня
int create_stone(struct Dino *dino, struct Field *field, Direction direction);

// Функция перемещения камня
int push_stone(struct Dino *dino, struct Field *field, Direction direction);

// Функция для обработки команды MAKE
int handle_make_command(struct Dino *dino, struct Field *field, const char *direction_str);

// Функция для обработки команды PUSH
int handle_push_command(struct Dino *dino, struct Field *field, const char *direction_str);

// Команды деревьев

// Функция создания дерева
int create_tree(struct Dino *dino, struct Field *field, Direction direction);

// Функция срубания дерева
int cut_tree(struct Dino *dino, struct Field *field, Direction direction);

// Функция для обработки GROW
int handle_grow_command(struct Dino *dino, struct Field *field, const char *direction_str);

//Функция для обработки CUT
int handle_cut_command(struct Dino *dino, struct Field *field, const char *direction_str);

// Вспомогательные команды для вычисления координат 

//Вычисляет координаты клетки рядом с динозавром в указанном направлении
void get_target_cell(struct Dino *dino, struct Field *field, Direction direction, int *target_x, int *target_y);

// Вычисляет координаты клетки из произвольной начальной точки
void get_target_cell_from(int start_x, int start_y, struct Field *field, Direction direction, int *target_x, int *target_y);
