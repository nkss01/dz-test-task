# dz-test-task 🦖
Тестовое задание состоит из трёх подзаданий. Каждое подзадание находится в своей папке. Ниже для каждого подзадания приведены описание и инструкция по развёртыванию.

## Задание 1
Программа написана на Си, используются только системные библиотеки. Поддерживает Windows и UNIX, но тестировалась только на Windows.
 - в папке **test1** находятся стандартные тестовые файлы;
 - в папке **test2** -- тестовые файлы для случая с циклической зависимостью;
 - папка **winx64** содержит собранный exe-файл программы для Windows;
 - папка **src** содержит исходный код.

Для сборки исходного кода нужен компилятор Си. Рекомендуется использовать gcc:

    gcc -w -o task1 task1.c
В Linux компилятор можно [установить через терминал](https://phoenixnap.com/kb/install-gcc-ubuntu).
На Windows можно установить через [MinGW](https://sourceforge.net/projects/mingw/) или использовать WSL. Если вы выбираете другой компилятор под Windows, без поддержки библиотек GNU (например, компилятор Visual Studio), перед компиляцией необходимо поместить в папку **src** файл **dirent.h** из директории winx64. Этот заголовочный файл содержит реализацию интерфейса dirent под Windows. Взят у [tronkko](https://github.com/tronkko/dirent).

После запуска программы введите полный или локальный путь до папки с текстовыми файлами и нажмите Enter. Либо передайте путь первым аргументом в командной строке или терминале.

## Задание 2
Серверная часть написана на C++ с использованием библиотек Boost и libpq, а также библиотеки [nlohmann::json](https://github.com/nlohmann/json) для работы с форматом JSON. БД использует PostgreSQL, клиентская часть написана на ванильном JS.
Тестировалось на Windows 11 и в браузере Opera.
 - в папке **winx64** -- собранный сервер для Windows со стандартным конфигом;
 - папка **src** содержит исходный код серверной части;
 - папка **web** содержит исходный код клиентской части.

Для сборки и запуска:
 1. установите PostgreSQL и создайте новую базу данных;
 2. в папке **src** перейдите в файл **config.h** и настройте конфиг: порт, максимальное количество соединений для пула запросов, строку для подключения к БД;
 3. установите Boost. В Linux можно [использовать терминал](https://stackoverflow.com/questions/12578499/how-to-install-boost-on-ubuntu), на Windows -- [vcpkg](https://vcpkg.io/), либо собрать самостоятельно [исходники](https://www.boost.org/users/download/) с официального сайта;
 4. установите libpq. В Linux -- через [терминал](https://askubuntu.com/questions/1372562/how-to-install-libpq-dev-14-0-1-on-ubuntu-21-10), на Windows -- через [vcpkg](https://vcpkg.io/);
 5. скомпилируйте сервер. Необходим компилятор с поддержкой c++17. Рекомендуется использовать g++. В Linux [устанавливается через терминал](https://onstartup.ru/razrabotka/g/), для Windows можно использовать [MinGW](https://sourceforge.net/projects/mingw/).
 Примерный вид команды:

    g++ -std=c++17 -I/usr/include -I/usr/local/include task2.cpp -o server -lboost_system -lboost_filesystem -lpqxx -lpq -lpthread

 7. запустите сервер;
 8. из папки **web** откройте в браузере страницу **index.html**

На странице введите адрес сервера и нажмите кнопку **Начать**. Если в БД не существует таблицы со студентами, будет предложено автоматически инициализировать её, после чего можно перейти к основному функционалу.

## Задание 3
Клиентская часть написана на ванильном JS. Также для обхода CORS создан прокси-сервер на NodeJS.
 - в папке **web** -- исходники клиента;
 - в папке **src** -- исходники сервера;

Перед запуском установите NodeJS, после чего в папке src выполните команду:

    node proxy.js
Если стандартный URL -- *localhost:3000* -- не подходит, перед запуском клиентской части отредактируйте первую строчку в файле **web\script.js**
После чего откройте в браузере страницу **web\index.html**

*(Примечание: по какой-то причине, так и не получилось воспользоваться методом /api/todos/date -- не приходят значения по диапазону дат, даже если выставить минимальное и максимальное возможные значения. Поэтому в текущей реализации задачи сортируются на стороне клиента. Что, конечно, не совсем корректно в полевых условиях. Если представить, что метода /api/todos/date не существует, то в идеальной реализации сортировку можно проводить на стороне прокси-сервера)*