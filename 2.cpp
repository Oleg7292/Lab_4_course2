#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>

using namespace std;

struct Student {
    string name;
    string gender;
    string faculty;
    string group;
    string exercise;
    int repetitions;
};

// Функция для генерации случайных значений для студентов
string randomString(const vector<string>& values) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, values.size() - 1);
    return values[dis(gen)];
}

// Функция для генерации случайного имени
string randomName() {
    static const vector<string> names = {
        "Alice", "Bob", "Clara", "David", "Eva", "Frank", "Grace", "Hank", "Ivy", "Jack", "Lily", "John", "Emma", "Luke", "Zoe"
    };
    return randomString(names);
}

// Генерация случайных данных студентов
void generateRandomStudents(int numStudents, vector<Student>& students) {
    static const vector<string> faculties = {"CS", "Math", "Physics", "Biology", "Chemistry"};
    static const vector<string> groups = {"G1", "G2", "G3"};
    static const vector<string> exercises = {"Squats", "Pushups", "Running", "Jumping", "Pullups"};
    static const vector<string> genders = {"F", "M"};

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> disReps(50, 150);  // Диапазон повторений от 50 до 150

    for (int i = 0; i < numStudents; ++i) {
        string name = randomName();
        string gender = randomString(genders);
        string faculty = randomString(faculties);
        string group = randomString(groups);
        string exercise = randomString(exercises);
        int repetitions = disReps(gen);

        students.push_back({name, gender, faculty, group, exercise, repetitions});
    }
}

// Функция для фильтрации студентов по полю и упражнению
void filterStudentsByGenderAndExercise(const vector<Student>& students, vector<Student>& result, const string& faculty, const string& exercise) {
    for (const auto& student : students) {
        if (student.faculty == faculty && student.exercise == exercise) {
            result.push_back(student);
        }
    }
}

// Функция для сортировки студентов по количеству повторений
void sortStudentsByRepetitions(vector<Student>& students) {
    sort(students.begin(), students.end(), [](const Student& a, const Student& b) {
        return a.repetitions > b.repetitions;
    });
}

// Функция для параллельной обработки данных (по полу и упражнению)
void processGenderAndExercise(const vector<Student>& students, vector<Student>& result, const string& gender, const string& faculty, const string& exercise) {
    filterStudentsByGenderAndExercise(students, result, faculty, exercise);
    sortStudentsByRepetitions(result);
}

// Функция для многопоточной обработки
void processInThread(const vector<Student>& students, vector<Student>& result, const string& gender, const string& faculty, const string& exercise) {
    processGenderAndExercise(students, result, gender, faculty, exercise);
}

int main() {
    int numThreads, numStudents;
    string faculty, exercise;

    cout << "Enter the number of students to generate: ";
    cin >> numStudents;

    cout << "Enter the number of threads: ";
    cin >> numThreads;

    cout << "Enter faculty and exercise for top list: ";
    cin >> faculty >> exercise;

    vector<Student> students;
    generateRandomStudents(numStudents, students);

    // Измеряем время с многопоточностью
    vector<Student> women, men;
    auto start = chrono::high_resolution_clock::now();

    thread womenThread(processInThread, ref(students), ref(women), "F", faculty, exercise);
    thread menThread(processInThread, ref(students), ref(men), "M", faculty, exercise);

    womenThread.join();
    menThread.join();

    auto end = chrono::high_resolution_clock::now();
    auto durationWithThreads = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Top 5 women in exercise " << exercise << ":" << endl;
    for (int i = 0; i < min(5, (int)women.size()); i++) {
        cout << women[i].name << " - " << women[i].repetitions << " reps" << endl;
    }

    cout << "Top 5 men in exercise " << exercise << ":" << endl;
    for (int i = 0; i < min(5, (int)men.size()); i++) {
        cout << men[i].name << " - " << men[i].repetitions << " reps" << endl;
    }

    cout << "Time with threads: " << durationWithThreads.count() << " ms" << endl;

    // Измеряем время без многопоточности
    women.clear();
    men.clear();

    start = chrono::high_resolution_clock::now();

    filterStudentsByGenderAndExercise(students, women, faculty, exercise);
    sortStudentsByRepetitions(women);

    filterStudentsByGenderAndExercise(students, men, faculty, exercise);
    sortStudentsByRepetitions(men);

    end = chrono::high_resolution_clock::now();
    auto durationWithoutThreads = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Time without threads: " << durationWithoutThreads.count() << " ms" << endl;

    return 0;
}