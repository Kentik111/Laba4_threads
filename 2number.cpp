#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <random>
#include <algorithm>
#include <atomic>

using namespace std;

// Структура для хранения информации о сотруднике
struct Employee {
    string fio;        // ФИО сотрудника
    string position;   // Должность
    string department; // Отдел
    double salary;     // Зарплата
};

// Векторы с данными для генерации случайных сотрудников
vector<string> surnames = {"Иванов", "Петров", "Сидоров", "Козлов", "Смирнов", "Кузнецов", "Попов", "Васильев", "Михайлов", "Новиков"};
vector<string> names = {"Иван", "Петр", "Сидор", "Козло", "Смирн", "Алексей", "Дмитрий", "Александр", "Михаил", "Николай"};
vector<string> patronymics = {"Иванович", "Петрович", "Сидорович", "Козлович", "Смирнович", "Алексеевич", "Дмитриевич", "Александрович", "Михайлович", "Николаевич"};
vector<string> positions = {"Инженер", "Менеджер", "Программист", "Аналитик", "Тестировщик"};
vector<string> departments = {"Отдел разработки", "Отдел продаж", "Отдел маркетинга", "Отдел финансов", "Отдел HR"};

// Функция для генерации случайных сотрудников
vector<Employee> generateEmployees(int count) {
    vector<Employee> employees;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> surnameDist(0, surnames.size() - 1);
    uniform_int_distribution<> nameDist(0, names.size() - 1);
    uniform_int_distribution<> patronymicDist(0, patronymics.size() - 1);
    uniform_int_distribution<> positionDist(0, positions.size() - 1);
    uniform_int_distribution<> departmentDist(0, departments.size() - 1);
    uniform_real_distribution<> salaryDist(30000, 100000);

    for (int i = 0; i < count; ++i) {
        Employee emp;
        emp.fio = surnames[surnameDist(gen)] + " " + names[nameDist(gen)] + " " + patronymics[patronymicDist(gen)];
        emp.position = positions[positionDist(gen)];
        emp.department = departments[departmentDist(gen)];
        emp.salary = salaryDist(gen);
        employees.push_back(emp);
    }

    return employees;
}

// Функция для расчета средней зарплаты по отделам (однопоточная версия)
map<string, double> calculateAverageSalary(const vector<Employee>& employees) {
    map<string, double> totalSalary; // Сумма зарплат по отделам
    map<string, int> count;          // Количество сотрудников по отделам

    // Проходим по всем сотрудникам и суммируем их зарплаты
    for (const auto& emp : employees) {
        totalSalary[emp.department] += emp.salary;
        count[emp.department]++;
    }

    map<string, double> averageSalary; // Средняя зарплата по отделам
    // Рассчитываем среднюю зарплату для каждого отдела
    for (const auto& pair : totalSalary) {
        averageSalary[pair.first] = pair.second / count[pair.first];
    }

    return averageSalary;
}

// Функция для вывода сотрудников, у которых зарплата выше средней по отделу
void printEmployeesAboveAverage(const vector<Employee>& employees, const map<string, double>& averageSalary) {
    // Проходим по всем сотрудникам и выводим тех, у кого зарплата выше средней по отделу
    for (const auto& emp : employees) {
        if (emp.salary > averageSalary.at(emp.department)) {
            cout << "ФИО: " << emp.fio << ", Должность: " << emp.position << ", Отдел: " << emp.department << ", Зарплата: " << emp.salary << endl;
        }
    }
}

// Функция для расчета средней зарплаты по отделам в многопоточной версии
void calculateAverageSalaryThread(const vector<Employee>& employees, map<string, atomic<double>>& totalSalary, map<string, atomic<int>>& count, int start, int end) {
    // Проходим по диапазону сотрудников, заданному параметрами start и end
    for (int i = start; i < end; ++i) {
        const auto& emp = employees[i];
        double currentSalary = totalSalary[emp.department].load(memory_order_relaxed);
        // Используем атомарные операции для безопасного добавления зарплаты
        while (!totalSalary[emp.department].compare_exchange_weak(currentSalary, currentSalary + emp.salary, memory_order_relaxed)) {}
        count[emp.department].fetch_add(1, memory_order_relaxed);
    }
}

int main() {
    int numEmployees = 10; // Количество сотрудников
    vector<Employee> employees = generateEmployees(numEmployees);

    // Время без многопоточности
    auto start = chrono::high_resolution_clock::now();
    auto averageSalarySingleThread = calculateAverageSalary(employees);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> durationSingleThread = end - start;
    cout << "Время обработки без многопоточности: " << durationSingleThread.count() << " сек" << endl;

    // Время с многопоточностью
    start = chrono::high_resolution_clock::now();
    map<string, atomic<double>> totalSalary; // Сумма зарплат по отделам (многопоточная версия)
    map<string, atomic<int>> count;          // Количество сотрудников по отделам (многопоточная версия)

    int numThreads = thread::hardware_concurrency(); // Количество доступных потоков
    vector<thread> threads;
    int chunkSize = employees.size() / numThreads;

    // Создаем потоки для обработки сотрудников
    for (int i = 0; i < numThreads; ++i) {
        int start = i * chunkSize;
        int end = (i == numThreads - 1) ? employees.size() : start + chunkSize;
        threads.emplace_back(calculateAverageSalaryThread, ref(employees), ref(totalSalary), ref(count), start, end);
    }

    // Дожидаемся завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    map<string, double> averageSalaryMultiThread; // Средняя зарплата по отделам (многопоточная версия)
    // Рассчитываем среднюю зарплату для каждого отдела
    for (const auto& pair : totalSalary) {
        averageSalaryMultiThread[pair.first] = pair.second / count[pair.first];
    }

    end = chrono::high_resolution_clock::now();
    chrono::duration<double> durationMultiThread = end - start;
    cout << "Время обработки с многопоточностью: " << durationMultiThread.count() << " сек" << endl;

    // Вывод результатов
    cout << "Результаты обработки без многопоточности:" << endl;
    printEmployeesAboveAverage(employees, averageSalarySingleThread);

    cout << "Результаты обработки с многопоточностью:" << endl;
    printEmployeesAboveAverage(employees, averageSalaryMultiThread);

    return 0;
}