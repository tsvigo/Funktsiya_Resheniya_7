#include "dialog.h"
#include "ui_dialog.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// надо заменить все long long и unsigned long long на библиотеку GMP
#include <QCoreApplication>
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <gmpxx.h>
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include "dialog.h"
//#include "ui_dialog.h"
//###########################################################################
#include <QDataStream>
#include <QDebug>
#include <QFile>

#include <QFileDialog>

#include <QProcess>
#include <fstream>
#include <iostream>

#include <QApplication>
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//###########################################################################
// переменные:
bool Odin_Uchitelia;
bool Odin_Programmi;
int var;
int neuron_index = 0, synapse_index = 0;

constexpr size_t NUM_SYNAPSES = 10105;

constexpr size_t NUM_NEYRONS = 205 ;
std::vector<mpz_class> list_of_neurons(0//NUM_NEYRONS
                                       );
std::vector<mpz_class> list_of_synapses(0//NUM_SYNAPSES
                                        );
//const mpz_class MAX_VALUE("18446744073709551615");
const std::string FILE_PATH = "/home/viktor/my_projects_qt_2/sgenerirovaty_sinapsi/random_sinapsi.bin";
 QString logFilePath =
    "/home/viktor/my_projects_qt_2_build/build-Funktsiya_Resheniya_7-Desktop_Qt_6_8_0-Debug/application.log";
//##################################################################


//###########################################################################
///////////////////////////
/////////////////// NOTE: функции: //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
     static QFile logFile("/home/viktor/my_projects_qt_2/Funktsiya_Resheniya_7/application.log");
     if (!logFile.isOpen()) {
         logFile.open(QIODevice::Append | QIODevice::Text);
     }
     QTextStream out(&logFile);
     QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

     switch (type) {
     case QtDebugMsg:
         out << timeStamp << " [DEBUG] " << msg << "\n";
         break;
     case QtInfoMsg:
         out << timeStamp << " [INFO] " << msg << "\n";
         break;
     case QtWarningMsg:
         out << timeStamp << " [WARNING] " << msg << "\n";
         break;
     case QtCriticalMsg:
         out << timeStamp << " [CRITICAL] " << msg << "\n";
         break;
     case QtFatalMsg:
         out << timeStamp << " [FATAL] " << msg << "\n";
         abort();
     }
     out.flush();
 }
 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 // перенаправляем вывод из консоли в лог файл
void redirectOutputToFile2(const QString &filePath) {
    // Открываем файл для записи и очищаем его содержимое
    FILE *file = freopen(filePath.toStdString().c_str(), "w", stdout);
    if (!file) {
        std::cerr << "Failed to redirect stdout to file" << std::endl;
        return;
    }

    // Перенаправляем stderr тоже, если нужно
    file = freopen(filePath.toStdString().c_str(), "w", stderr);
    if (!file) {
        std::cerr << "Failed to redirect stderr to file" << std::endl;
        return;
    }

    std::cout << "Console output is now redirected to the log file." << std::endl;
    std::cerr << "Error output is also redirected to the log file." << std::endl;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void writeVectorToFile(const std::vector<mpz_class>& vec, const std::string& filename) {
    FILE* outFile = fopen(filename.c_str(), "wb");
    if (!outFile) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }

    for (const auto& num : vec) {
        mpz_out_raw(outFile, num.get_mpz_t());
    }

    fclose(outFile);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<mpz_class> readVectorFromFile(const std::string& filename) {
   // std::vector<mpz_class> vec;
 //  std::vector<mpz_class> list_of_synapses;
    // list_of_synapses
    FILE* inFile = fopen(filename.c_str(), "rb");
    if (!inFile) {
        std::cerr << "Error opening file for reading." << std::endl;
     //   return list_of_synapses;
    }

    while (!feof(inFile)) {
        mpz_class num;
        if (mpz_inp_raw(num.get_mpz_t(), inFile) == 0) {
            break; // EOF or error
        }
        list_of_synapses.push_back(num);
    }

    fclose(inFile);
    return list_of_synapses;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vectorsAreEqual(const std::vector<mpz_class>& vec1, const std::vector<mpz_class>& vec2) {
    if (vec1.size() != vec2.size()) {
        return false;
    }
    for (size_t i = 0; i < vec1.size(); ++i) {
        if (vec1[i] != vec2[i]) {
            return false;
        }
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// arctg
mpz_class arctgActivation(int var) {
    // Преобразуем значение в mpf_class для вычисления арктангенса
    mpf_class value_f(list_of_neurons.at(var));

    // Вычисляем арктангенс
    mpf_class arctg_value = atan(value_f.get_d());

    // Преобразуем обратно в mpz_class (здесь может потребоваться округление)
    mpz_class result(arctg_value);

    // Обновляем значение в векторе
    list_of_neurons[var] = result;

    return list_of_neurons.at(var);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// сигмоида
mpz_class sigmoid_activation_function(int var) {
    mpz_class base = 3;
    mpz_class exponentiated_value;

    // Проверка, что значение не отрицательное
    if (list_of_neurons.at(var) < 0) {
        throw std::runtime_error("Negative exponent not supported.");
    }

    // Проверка, что значение не слишком большое
    if (list_of_neurons.at(var) > 1000) { // Example threshold
        throw std::runtime_error("Exponent too large, may cause overflow.");
    }

    // Step 1: Compute 3^list_of_neurons[var]
    mpz_pow_ui(exponentiated_value.get_mpz_t(), base.get_mpz_t(), list_of_neurons.at(var).get_ui());

    // Step 2: Compute 1 + 3^list_of_neurons[var]
    mpz_class denominator = 1 + exponentiated_value;

    // Step 3: Compute the inverse 1 / (1 + 3^list_of_neurons[var])
    mpz_class numerator = 1;
    mpz_class result;

    // We will compute the result as numerator / denominator
    mpz_tdiv_q(result.get_mpz_t(), numerator.get_mpz_t(), denominator.get_mpz_t());

    // Step 4: Update the list_of_neurons with the computed result
    list_of_neurons[var] = result;

    return list_of_neurons.at(var);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// квадратичная активация
mpz_class quadraticActivation(int var) {

    list_of_neurons[var] = list_of_neurons[var] * list_of_neurons[var];

    return (  list_of_neurons.at(var));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Функция для чтения чисел-синапсов из бинарного файла

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// void readFromFile(std::vector<mpz_class>& list_of_synapses, const std::string& filePath) {
//     std::ifstream inFile(filePath, std::ios::binary);
//     if (!inFile) {
//         qCritical() << "Ошибка открытия файла для чтения";
//         return;
//     }

//     for (size_t i = 0; i < NUM_SYNAPSES; ++i) {
//         size_t size;
//         inFile.read(reinterpret_cast<char*>(&size), sizeof(size));
//         std::vector<char> buffer(size);
//         inFile.read(buffer.data(), size);
//         list_of_synapses[i].set_str(std::string(buffer.begin(), buffer.end()), 10);
//     }

//     inFile.close();
// }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// void readFromFile(std::vector<mpz_class>& list_of_synapses, const std::string& filePath) {
//     std::ifstream inFile(filePath, std::ios::binary);
//     if (!inFile) {
//         qCritical() << "Ошибка открытия файла для чтения";
//         return;
//     }

//     // Убедимся, что вектор имеет нужный размер
//     list_of_synapses.resize(NUM_SYNAPSES);

//     for (size_t i = 0; i < NUM_SYNAPSES; ++i) {
//         quint32 size;
//         inFile.read(reinterpret_cast<char*>(&size), sizeof(size));
//         if (inFile.gcount() != sizeof(size)) {
//             qCritical() << "Ошибка чтения размера числа из файла";
//             return;
//         }

//         std::vector<unsigned char> buffer(size);
//         inFile.read(reinterpret_cast<char*>(buffer.data()), size);
//         if (inFile.gcount() != size) {
//             qCritical() << "Ошибка чтения числа из файла";
//             return;
//         }

//         // Импортируем данные в mpz_class
//         mpz_import(list_of_synapses[i].get_mpz_t(), size, 1, 1, 0, 0, buffer.data());
//     }

//     inFile.close();
// }
void readFromFile(std::vector<mpz_class>& list_of_synapses, const std::string& filePath) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        qCritical() << "Ошибка открытия файла для чтения";
        return;
    }

    // Убедимся, что вектор имеет нужный размер
    list_of_synapses.resize(NUM_SYNAPSES);

    for (size_t i = 0; i < NUM_SYNAPSES; ++i) {
        quint32 size;
        inFile.read(reinterpret_cast<char*>(&size), sizeof(size));
        if (inFile.gcount() != sizeof(size)) {
            qCritical() << "Ошибка чтения размера числа из файла для элемента" << i;
            return;
        }

        if (size == 0) {
            qCritical() << "Размер числа равен 0 для элемента" << i;
            return;
        }

        std::vector<unsigned char> buffer(size);
        inFile.read(reinterpret_cast<char*>(buffer.data()), size);
        if (inFile.gcount() != size) {
            qCritical() << "Ошибка чтения числа из файла для элемента" << i;
            qCritical() << "Ожидаемый размер:" << size << ", прочитанный размер:" << inFile.gcount();
            return;
        }

        // Импортируем данные в mpz_class
        mpz_import(list_of_synapses[i].get_mpz_t(), size, 1, 1, 0, 0, buffer.data());
    }

    inFile.close();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readFromFile2(std::vector<mpz_class>& list_of_synapses, const std::string& filePath) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        qCritical() << "Ошибка открытия файла для чтения";
        return;
    }

    for (size_t i = 0; i < NUM_SYNAPSES; ++i) {
        size_t size;
        inFile.read(reinterpret_cast<char*>(&size), sizeof(size));
        std::vector<char> buffer(size);
        inFile.read(buffer.data(), size);
        list_of_synapses[i].set_str(std::string(buffer.begin(), buffer.end()), 10);
    }

    inFile.close();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void printVector(const std::vector<mpz_class>& list_of_synapses) {
    int i=0;
    for (const auto& value : list_of_synapses) {
        qDebug() <<i<< ":"<< QString::fromStdString(value.get_str());
        i++;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//########################################################################
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
mpz_class activationFunction( // long long list_of_neurons.at(var)
    int var
    )
{
    //  mpz_class base, exponent, result;

    mpz_class giperparametr =2;//0;//2;//200;
    // base=3; /// тут скорее 3 потому что 2,7
    // exponent = list_of_neurons.at(var); // степень
    // result = pow(base, exponent);
    // Инициализация больших целых чисел
    mpz_class base("3");
    mpz_class exp(list_of_neurons.at(var));
    mpz_class mod("4611686018000000000");
    mpz_class result;
    // Вычисление (base ^ exp) % mod
    mpz_powm(result.get_mpz_t(), base.get_mpz_t(), exp.get_mpz_t(), mod.get_mpz_t());
    // mpz_powm ( result,  3,  exponent, 4611686018000000000);
    if (list_of_neurons.at(var) <= 0)
        list_of_neurons.at(var) =list_of_neurons.at(var) * giperparametr * (result - 1);
    return (  list_of_neurons.at(var));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE: activationFunction_Bent_identity
mpz_class activationFunction_Bent_identity( // long long list_of_neurons.at(var)
    int var
    )
{
//     mpz_t n, root;
//  unsigned long int k = 2; // степень корня
//         mpz_root(root, n, k); // n - из чего извлекаем корень
//  // Использование функции mpz_set для присваивания
//  //mpz_t another;
// // mpz_init(list_of_neurons.at(var));
// // mpz_set(another, root); // Правильное присваивание
//  //  void mpz_powm (mpz_t rop, const mpz_t base, const mpz_t exp, const mpz_t mod)

//  n=(list_of_neurons.at(var) * list_of_neurons.at(var))+1;

//     list_of_neurons.at(var) =(list_of_neurons.at(var) * list_of_neurons.at(var))+1
   return (  list_of_neurons.at(var));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE: bent_identity_activation объявление
mpz_class bent_identity_activation(size_t var) {
    if (var >= list_of_neurons.size()) {
        throw std::out_of_range("Index out of range");
    }

    mpz_class x = list_of_neurons.at(var);
    mpz_class x_squared, one, sqrt_value, num, den, result_value;

    // x^2
    mpz_mul(x_squared.get_mpz_t(), x.get_mpz_t(), x.get_mpz_t());

    // x^2 + 1
    one = 1;
    mpz_add(x_squared.get_mpz_t(), x_squared.get_mpz_t(), one.get_mpz_t());

    // sqrt(x^2 + 1)
    mpz_sqrt(sqrt_value.get_mpz_t(), x_squared.get_mpz_t());

    // sqrt(x^2 + 1) - 1
    mpz_sub(sqrt_value.get_mpz_t(), sqrt_value.get_mpz_t(), one.get_mpz_t());

    // (sqrt(x^2 + 1) - 1) / 2
    den = 2;
    mpz_fdiv_q(num.get_mpz_t(), sqrt_value.get_mpz_t(), den.get_mpz_t());

    // (sqrt(x^2 + 1) - 1) / 2 + x
    mpz_add(result_value.get_mpz_t(), num.get_mpz_t(), x.get_mpz_t());

    return result_value;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//const QString RANDOM_NUMBERS_FILE_NAME = "random_numbers.bin";

std::vector<mpz_class> readRandomNumbersFromFile(const QString& fileName) {
    std::vector<mpz_class> numbers;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cerr << "Failed to open random numbers file" << std::endl;
        return numbers;
    }
    QDataStream in(&file);
    while (!in.atEnd() && numbers.size() < 205) {
        QByteArray byteArray;
        in >> byteArray;
        numbers.push_back(mpz_class(byteArray.constData()));
    }
    file.close();
    return numbers;
}

void printVector2(const std::vector<mpz_class>& vec) {
    for (const auto& number : vec) {
        std::cout << number.get_str() << std::endl;
    }
}



// конец объявлений функций
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    // Регистрация пользовательского обработчика сообщений
    qInstallMessageHandler(customMessageHandler);
    ui->setupUi(this);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //########################################################################################################
    std::cout << "Funktsiya_Resheniya_7" << std::endl;
    //########################################################################################################
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// logFilePath
  //  redirectOutputToFile2(logFilePath);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // читаем синапсы из файла в вектор
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*

    //    std::vector<mpz_class> read_synapses(NUM_SYNAPSES);
    // readFromFile2(list_of_synapses
    //              //read_synapses
    //              , FILE_PATH);
   // NOTE: readVectorFromFile опреДЕЛЕНИЕ
    readVectorFromFile(FILE_PATH);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // readFromFile(list_of_synapses, FILE_PATH);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    qDebug() << "Размер list_of_synapses:" << list_of_synapses.size();
    std::cout << "конец чтения синапсов в вектор" << std::endl;

    // Вывод значений вектора в консоль
    printVector(list_of_synapses
          // read_synapses
                   );
     // printVector(list_of_synapses);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::cout << "//"
                 "#################################################################################"
                 "#######################"
              << std::endl;
    //###########################################################################
    //////////////////// считали синапсы в вектор //////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // читаем нейроны в вектор

    // Вызов диалога выбора файла
    QString fileName_neyroni = QFileDialog::getOpenFileName(nullptr, "Выберите файл",
                                                            //"/home/viktor/1_rukoy/"
                                                            "/home/viktor/1_rukoy_GMP/"
                                                            ,  "bin Files (*.bin)");

    // Проверка, был ли файл выбран
    if (!fileName_neyroni.isEmpty()) {
        qDebug() << "Выбранный файл:" << fileName_neyroni;
    } else {
        qDebug() << "Файл не был выбран.";
    }
    // Преобразование QString в std::string
    std::string stdFileName_neyroni = fileName_neyroni.toStdString();
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // читаем нейроны из файла в вектор
    // Чтение файла с 205 случайными числами и преобразование в вектор mpz_class
    //   std::vector<mpz_class>
    list_of_neurons = readRandomNumbersFromFile(
        //  RANDOM_NUMBERS_FILE_NAME
        fileName_neyroni
        );
    // Вывод вектора в консоль
    // printVector2(//randomVector
    //       list_of_neurons       );
    //###########################################################################
    ////////////////////////// считали нейроны в вектор ////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*

    qDebug() << "Размер list_of_neurons:" << list_of_neurons.size();
    std::cout << "конец чтения нейронов в вектор" << std::endl;


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Проверка содержимого вектора
    // ...
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//################################ NOTE: блок решения
//###########################################
    // блок вычисления-решения 200 нейрона
    // проверка - решение
    for (var = 100; // первый for
         var < 200; //200;
         ++var)     // This is the range of neurons
    {
   //     bool increase = true; // Флаг для чередования увеличения и уменьшения
        //    if (list_of_neurons->at(200)<0) break;
        for (neuron_index = 0, synapse_index = 0;  // второй for

             /*,*/ synapse_index < 10100 //, neuron_index < 200
             // при включении условия выше 200 нейрон меняется
             ;
             ++neuron_index,
             synapse_index = synapse_index + 100 // вроде тут ошибка
             )

        {

            try
            {
                 list_of_neurons.at(var)
                                       = list_of_neurons.at(var)
                   + ((list_of_neurons.at(neuron_index)

                        -                                     // вычитаем
                        list_of_synapses.at(synapse_index)));

   // NOTE: выведем все результаты

     qDebug() << "list_of_neurons.at("<<var<<")=list_of_neurons.at("<<var<<")+ ((list_of_neurons.at("
                          << neuron_index<<")-list_of_synapses.at("<<synapse_index<<")";
 // qDebug() << list_of_neurons.at(var)<<"="<<list_of_neurons.at(var)<<"+"<< list_of_neurons.at(neuron_index)<<"-"<<list_of_synapses.at(synapse_index);
 qDebug() <<  QString::fromStdString(list_of_neurons.at(var).get_str())
     <<"="<< QString::fromStdString(list_of_neurons.at(var).get_str())<<"+"<<
         QString::fromStdString( list_of_neurons.at(neuron_index).get_str())<<"-"<<
       QString::fromStdString( list_of_synapses.at(synapse_index).get_str())
      ;
  // QString::fromStdString(list_of_neurons.at(index).get_str());
     //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // mpz_class delta = list_of_neurons.at(neuron_index) - list_of_synapses.at(synapse_index);
                // if (increase) {
                //     list_of_neurons.at(var) += delta;
                // } else {
                //     list_of_neurons.at(var) -= delta;
                // }
                // increase = !increase; // Чередование флага

            }
            catch (const std::out_of_range &e)
            {
                //  std::cerr << "Caught an exception: " << e.what() << '\n';
            }

        } // второй for
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // тут видимо умножать на функцию активации
        //  activationFunction(var)
// list_of_neurons.at(var)=list_of_neurons.at(var)*activationFunction(var);
        // NOTE опрЕДЕЛЕНИЕ ФУНКЦИИ АКТИВАЦИИ 1
        // Вычисление активации "Bent identity"
      //  std::vector<mpz_class> activated_neurons = bent_identity_activation(list_of_neurons);
   //  list_of_neurons.at(var)=list_of_neurons.at(var)*bent_identity_activation(var);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
list_of_neurons.at(var)=arctgActivation( var)  ;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
     //   list_of_neurons.at(var)=quadraticActivation (var);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    } // первый for
    //////////////////////
    for (int neuron_index = 100, synapse_index = 10000; // второй for

         synapse_index < 10100;
         ++neuron_index, ++synapse_index)
    {

      //  bool increase = true; // Флаг для чередования увеличения и уменьшения

              list_of_neurons.at(200)
                  = list_of_neurons.at(200)
              + ((list_of_neurons.at(neuron_index)
                                 -                                     // вычитание
                  list_of_synapses.at(synapse_index)));
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
              // NOTE: выведем все результаты

              qDebug() << "list_of_neurons.at("<<200<<")=list_of_neurons.at("<<200<<")+ ((list_of_neurons.at("
                       << neuron_index<<")-list_of_synapses.at("<<synapse_index<<")";
                           qDebug() <<  QString::fromStdString(list_of_neurons.at(200).get_str())
                       <<"="<< QString::fromStdString(list_of_neurons.at(200).get_str())<<"+"<<
                  QString::fromStdString( list_of_neurons.at(neuron_index).get_str())<<"-"<<
                  QString::fromStdString( list_of_synapses.at(synapse_index).get_str())
                  ;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*              mpz_class delta = list_of_neurons.at(neuron_index) - list_of_synapses.at(synapse_index);
              if (increase) {
                  list_of_neurons.at(200) += delta;
              } else {
                  list_of_neurons.at(200) -= delta;
              }
              increase = !increase; */// Чередование флага

    } // for
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // тут видимо умножать на функцию активации
    //  activationFunction(var)
// list_of_neurons.at(200)=list_of_neurons.at(200)*activationFunction(200);
    // NOTE: определение функции активации 2
//      list_of_neurons.at(200)=list_of_neurons.at(200)*bent_identity_activation(200);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
list_of_neurons.at(200)=arctgActivation( 200)  ;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 //   list_of_neurons.at(200)=quadraticActivation (200);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //####### конец вычисления 200 нейрона ####################################################################
    /////////////   показываем что определила программа после решения
    // Почему же решение меняет 200 нейрон?
    if
        //  ( variable_error <=0)
        (list_of_neurons.at(200) < 0)

    {
        // ui->label->setText("Программа считает что это 1.");
        std::cout << "Программа считает что это 1." << std::endl;
        Odin_Programmi = true;
        // std::cout << "Программа остановлена. Ошибки в форматах синапсов или нейронов."<< std::endl;
    }
    //         else
    if (list_of_neurons.at(200) >= 0) {
        //  ui->label->setText("Программа считает что это не 1.");
        // тут уже ненормально показывает - как будто при решении меняются нейроны/синапсы
        std::cout << "Программа считает что это не 1." << std::endl;
        Odin_Programmi = false;
    }
    //########################################################################################################
    std::cout << "(после решения): list_of_neurons->at(200) = "
              << list_of_neurons.at(200) << std::endl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_clicked()
{
    // выбор программы обучения:
    if ( Odin_Programmi==false) // Если не распознана 1:
    {
        Odin_Uchitelia=true;
        ui->label_2->setText ("Odin_Programmi==false; Odin_Uchitelia=true");
        // cycle_of_distinguishing_a_one_with_vectors_GUI
        QProcess::startDetached(
            "/home/viktor/my_projects_qt_2_build/build-1_v_3_Widget-Desktop_Qt_6_8_0-Release/1_v_3_Widget"
         //   "/home/viktor/my_projects_qt_2_build/build-1_v_2-Desktop-Release/1_v_2"
            //   "/home/viktor/my_projects_qt_2_build/build-cycle_of_distinguishing_a_one_with_vectors_GUI_2_uu-Desktop_Qt_5_12_12_GCC_64bit-Release/cycle_of_distinguishing_a_one_with_vectors_GUI_2_uu"
            , qApp->arguments());
        //   qApp->quit();
    }
    else // Если не распознана не 1:
    {
        Odin_Uchitelia=false;
        ui->label_2->setText ("Odin_Programmi==true; Odin_Uchitelia=false");
        // bez_1
        QProcess::startDetached(

            //  "/home/viktor/my_projects_qt_2_build/build-bez_1_GUI_3_uu-Desktop_Qt_5_12_12_GCC_64bit-Release/bez_1_GUI_3_uu"
          //  "/home/viktor/my_projects_qt_2_build/build-bez_1-Desktop-Release/bez_1"
        "/home/viktor/my_projects_qt_2_build/build-bez_1_GMP-Desktop_Qt_6_8_0-Release/bez_1_GMP"
            , qApp->arguments());
        //          qApp->quit();
    }
}

