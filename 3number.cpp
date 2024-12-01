#include <iostream>
#include <vector>
using namespace std;

// Структура для представления одномерного массива (вектора) как связного списка
struct Node {
    int value;
    Node* next;
};

struct Vector {
    Node* head;
    Vector* next;
    int size;
};

// Создаем новый вектор заданного размера
Vector* NewVector(int size) {
    Vector* v = new Vector{nullptr, nullptr, size};
    Node* current = new Node{0, nullptr}; // Первый узел
    v->head = current;

    // Создаем цепочку из узлов с нулевыми значениями
    for (int i = 1; i < size; i++) {
        Node* newNode = new Node{0, nullptr};
        current->next = newNode;
        current = newNode;
    }
    return v;
}

// Получаем значение элемента вектора по индексу
int Get(Vector* v, int index, bool& ok) {
    if (index < 0 || index >= v->size) {
        ok = false; // Индекс вне границ
        return 0;
    }
    Node* current = v->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    ok = true;
    return current->value;
}

// Устанавливаем значение элемента вектора по индексу
bool Set(Vector* v, int value, int index) {
    if (index < 0 || index >= v->size) {
        return false; // Индекс вне границ
    }
    Node* current = v->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    current->value = value;
    return true;
}

// Выводим вектор на экран
void Print(Vector* v) {
    Node* current = v->head;
    while (current != nullptr) {
        cout << current->value << " ";
        current = current->next;
    }
    cout << endl;
}

// Структура матрицы (двумерного массива) с заданными размерами и инициализируем нулями
// Как связный список векторов
struct Matrix {
    Vector* head;
    int rows;
    int cols;
};

// Создаем новую матрицу
Matrix* NewMatrix(int rows, int cols) {
    Matrix* matrix = new Matrix{nullptr, rows, cols};
    Vector* currentVector = NewVector(cols);
    matrix->head = currentVector;

    // Создаем связанный список векторов для строк матрицы
    for (int i = 1; i < rows; i++) {
        Vector* newVector = NewVector(cols);
        currentVector->next = newVector;
        currentVector = newVector;
    }
    return matrix;
}

// Устанавливаем значение элемента матрицы
bool SetElement(Matrix* matrix, int row, int col, int value) {
    if (row >= matrix->rows || col >= matrix->cols || col < 0 || row < 0) {
        return false; // Индекс вне границ
    }

    Vector* currentVector = matrix->head;
    for (int i = 0; i < row; i++) {
        currentVector = currentVector->next;
    }
    return Set(currentVector, value, col);
}

// Получаем значение элемента матрицы
int GetElement(Matrix* matrix, int row, int col, bool& ok) {
    if (row >= matrix->rows || col >= matrix->cols || col < 0 || row < 0) {
        ok = false; // Индекс вне границ
        return 0;
    }

    Vector* currentVector = matrix->head;
    for (int i = 0; i < row; i++) {
        currentVector = currentVector->next;
    }
    return Get(currentVector, col, ok);
}

// Выводим матрицу на экран
void Print(Matrix* matrix) {
    Vector* currentVector = matrix->head;
    for (int i = 0; i < matrix->rows; i++) {
        Node* currentRow = currentVector->head;
        for (int j = 0; j < matrix->cols; j++) {
            cout << currentRow->value << " ";
            currentRow = currentRow->next;
        }
        cout << endl;
        currentVector = currentVector->next;
    }
}

// Структура банка
// Банк ресурсов
struct Bank {
    Matrix* max; // матрица максимальных потребностей
    Matrix* alloc; // матрица выделенных ресурсов
    Vector* avail; // вектор доступных ресурсов
    int numProcesses; // количество процессов
    int numResources; // количество ресурсов
};

// Создаем новый банк с заданными ресурсами
Bank* NewBank(Matrix* max, Matrix* alloc, Vector* avail, int numProcesses, int numResources) {
    return new Bank{max, alloc, avail, numProcesses, numResources};
}

// Обрабатываем запрос ресурсов от процесса
bool requestResources(Bank* bank, int process, Vector* request) {
    // Проверяем, не превышает ли запрос максимальные потребности процесса и достаточно ли доступных ресурсов
    for (int i = 0; i < bank->numResources; i++) {
        bool ok;
        int req = Get(request, i, ok); // запрошенные ресурсы
        int alloc = GetElement(bank->alloc, process, i, ok); // выделенные ресурсы
        int max = GetElement(bank->max, process, i, ok); // максимальные потребности
        if (req + alloc > max) {
            cout << "Запрос превышает максимальные потребности процессора для ресурса " << i << endl;
            return false; // Запрос превышает максимальные потребности
        }
    }
    // Проверка, достаточно ли доступных ресурсов для выполнения запроса
    for (int i = 0; i < bank->numResources; i++) {
        bool ok;
        int req = Get(request, i, ok); // запрошенные ресурсы
        int avail = Get(bank->avail, i, ok); // доступные ресурсы
        if (req > avail) {
            cout << "Недостаточно доступных ресурсов процессора для ресурса " << i << endl;
            return false; // Недостаточно доступных ресурсов
        }
    }
    // Выделение ресурсов, если проверки пройдены
    for (int i = 0; i < bank->numResources; i++) {
        bool ok;
        int req = Get(request, i, ok); // получаем значение запрошенного ресурса
        int alloc = GetElement(bank->alloc, process, i, ok); // получаем значение выделенных ресурсов для процесса
        SetElement(bank->alloc, process, i, alloc + req); // устанавливаем новое значение выделенных ресурсов для процесса
        int avail = Get(bank->avail, i, ok); // получаем значение доступного ресурса
        Set(bank->avail, avail - req, i); // устанавливаем новое значение доступных ресурсов для процесса
    }

    return true;
}

// Проверка, является ли текущее состояние системы безопасным
pair<bool, vector<int>> isSafeState(Bank* bank) {
    // Создаем вектор work, который будет использоваться для отслеживания доступных ресурсов
    Vector* work = NewVector(bank->numResources);
    // Инициализируем вектор work текущими доступными ресурсами
    for (int i = 0; i < bank->numResources; i++) {
        bool ok;
        int avail = Get(bank->avail, i, ok);
        Set(work, avail, i);
    }

    // Создаем массив finish, который будет отслеживать, завершены ли процессы
    vector<bool> finish(bank->numProcesses, false);
    // Создаем массив safeSequence, который будет хранить безопасную последовательность процессов
    vector<int> safeSequence;

    // Основной цикл для поиска безопасной последовательности
    while (true) {
        bool found = false;
        // Проходим по всем процессам
        for (int i = 0; i < bank->numProcesses; i++) {
            // Если процесс еще не завершен
            if (!finish[i]) {
                bool canFinish = true;
                // Проверяем, может ли процесс завершиться с текущими доступными ресурсами
                for (int j = 0; j < bank->numResources; j++) {
                    bool ok;
                    int need = GetElement(bank->max, i, j, ok) - GetElement(bank->alloc, i, j, ok);
                    int workVal = Get(work, j, ok);
                    // Если потребность процесса превышает доступные ресурсы, процесс не может завершиться
                    if (need > workVal) {
                        canFinish = false;
                        break;
                    }
                }
                // Если процесс может завершиться
                if (canFinish) {
                    // Освобождаем ресурсы, выделенные процессу
                    for (int j = 0; j < bank->numResources; j++) {
                        bool ok;
                        int alloc = GetElement(bank->alloc, i, j, ok);
                        int workVal = Get(work, j, ok);
                        Set(work, workVal + alloc, j);
                    }
                    // Помечаем процесс как завершенный
                    finish[i] = true;
                    // Добавляем процесс в безопасную последовательность
                    safeSequence.push_back(i);
                    found = true;
                }
            }
        }
        // Если не найдено ни одного процесса, который может завершиться, выходим из цикла
        if (!found) {
            break;
        }
    }

    // Проверяем, все ли процессы завершены
    for (int i = 0; i < bank->numProcesses; i++) {
        if (!finish[i]) {
            return {false, {}};
        }
    }

    // Возвращаем true и безопасную последовательность, если все процессы могут завершиться
    return {true, safeSequence};
}

int main() {
    int numProcesses = 5;
    int numResources = 3;
    Matrix* max = NewMatrix(numProcesses, numResources);
    SetElement(max, 0, 0, 7);
    SetElement(max, 0, 1, 5);
    SetElement(max, 0, 2, 3);
    SetElement(max, 1, 0, 3);
    SetElement(max, 1, 1, 2);
    SetElement(max, 1, 2, 2);
    SetElement(max, 2, 0, 9);
    SetElement(max, 2, 1, 0);
    SetElement(max, 2, 2, 2);
    SetElement(max, 3, 0, 2);
    SetElement(max, 3, 1, 2);
    SetElement(max, 3, 2, 2);
    SetElement(max, 4, 0, 4);
    SetElement(max, 4, 1, 3);
    SetElement(max, 4, 2, 3);

    Matrix* alloc = NewMatrix(numProcesses, numResources);
    SetElement(alloc, 0, 0, 0);
    SetElement(alloc, 0, 1, 1);
    SetElement(alloc, 0, 2, 0);
    SetElement(alloc, 1, 0, 2);
    SetElement(alloc, 1, 1, 0);
    SetElement(alloc, 1, 2, 0);
    SetElement(alloc, 2, 0, 3);
    SetElement(alloc, 2, 1, 0);
    SetElement(alloc, 2, 2, 2);
    SetElement(alloc, 3, 0, 2);
    SetElement(alloc, 3, 1, 1);
    SetElement(alloc, 3, 2, 1);
    SetElement(alloc, 4, 0, 0);
    SetElement(alloc, 4, 1, 0);
    SetElement(alloc, 4, 2, 2);

    Vector* avail = NewVector(numResources);
    Set(avail, 3, 0);
    Set(avail, 3, 1);
    Set(avail, 2, 2);

    Bank* bank = NewBank(max, alloc, avail, numProcesses, numResources);

    cout << "Максимальные ресурсы (Max):" << endl;
    Print(max);
    cout << "Выделенные ресурсы (Alloc):" << endl;
    Print(alloc);
    cout << "Доступные ресурсы (Avail):" << endl;
    Print(avail);

    Vector* request = NewVector(numResources);
    Set(request, 0, 0);
    Set(request, 0, 1);
    Set(request, 1, 2);

    cout << "Процесс " << 1 << " запрашивает ресурсы: ";
    Print(request);

    bool success = requestResources(bank, 1, request);
    if (success) {
        cout << "Новое состояние после выделения ресурсов:" << endl;
        cout << "Выделенные ресурсы (Alloc):" << endl;
        Print(alloc);
        cout << "Доступные ресурсы (Avail):" << endl;
        Print(avail);

        // Проверка на безопасное состояние и вывод безопасного пути
        auto [isSafe, safeSequence] = isSafeState(bank);
        if (isSafe) {
            cout << "Система находится в безопасном состоянии." << endl;
            cout << "Безопасный путь:" << endl;
            for (int process : safeSequence) {
                cout << "Процесс " << process << " -> ";
            }
            cout << endl;
        } else {
            cout << "Система находится в небезопасном состоянии." << endl;
        }
    } else {
        cout << "Состояние системы не изменилось." << endl;
    }

    return 0;
}