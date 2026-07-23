# Drogon Task Manager

Небольшой учебный проект для демонстрации возможностей веб-фреймворка
[Drogon](https://github.com/drogonframework/drogon):

- REST API с методами `GET`, `POST`, `PUT` и `DELETE`;
- получение и отправка JSON;
- проверка API-ключа через middleware;
- WebSocket-подключение;
- пользовательский плагин;
- простой фронтенд без сторонних библиотек.

## Требования

- CMake 3.16 или новее;
- компилятор с поддержкой C++17;
- установленная библиотека Drogon.

На macOS Drogon можно установить через Homebrew:

```bash
brew install drogon
```

## Сборка и запуск

```bash
cmake -S . -B build
cmake --build build
./build/drogontaskmanager
```

После запуска интерфейс доступен по адресу:

```text
http://localhost:8080/
```

## REST API

Все запросы к API должны содержать заголовок:

```text
X-API-Key: secret123
```

| Метод | Маршрут | Действие |
|---|---|---|
| `GET` | `/api/tasks` | Получить список задач |
| `POST` | `/api/tasks` | Создать задачу |
| `PUT` | `/api/tasks/{id}` | Изменить задачу |
| `DELETE` | `/api/tasks/{id}` | Удалить задачу |

Пример создания задачи:

```bash
curl -X POST http://localhost:8080/api/tasks \
  -H "X-API-Key: secret123" \
  -H "Content-Type: application/json" \
  -d '{"title":"Изучить Drogon","completed":false}'
```

Пример получения списка:

```bash
curl http://localhost:8080/api/tasks \
  -H "X-API-Key: secret123"
```

## Хранение данных

Задачи хранятся в `std::vector` в оперативной памяти. После остановки или
перезапуска сервера список задач очищается. Это сделано намеренно, чтобы
учебный пример оставался небольшим и не требовал настройки базы данных.

## Структура проекта

```text
controllers/   REST-контроллер
filters/       middleware для проверки API-ключа
models/        структура задачи и хранилище в памяти
plugins/       демонстрационный плагин
websockets/    WebSocket-контроллер
static/        HTML, CSS и JavaScript фронтенда
main.cc        точка входа
config.json    конфигурация Drogon
```
