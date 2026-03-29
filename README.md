# Clipboard Manager

Менеджер буфера обмена для Linux (Wayland/KDE) на Qt6 с минималистичным полупрозрачным интерфейсом.

## Возможности

- 📋 Сохранение истории буфера обмена (текст и изображения)
- 🔍 Быстрый поиск по содержимому
- 🎨 Минималистичный полупрозрачный интерфейс
- ⌨️ Навигация стрелками и горячими клавишами (⌘1-⌘9)
- 📱 Иконка в системном трее
- ⚡ Глобальная горячая клавиша Ctrl+Shift+F
- 💾 Сохранение истории между запусками (SQLite)
- 🖼️ Поддержка изображений

## Сборка

### Зависимости

```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-base-dev-tools cmake build-essential

# Fedora
sudo dnf install qt6-qtbase-devel cmake gcc-c++

# Arch Linux
sudo pacman -S qt6-base cmake base-devel
```

### Компиляция

```bash
cd clipboard-manager
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Установка

```bash
sudo make install
```

## Использование

### Запуск

```bash
./ClipboardManager
```

### Горячие клавиши

- `Ctrl+Shift+F` - Показать/скрыть окно
- `↑/↓` - Навигация по элементам
- `Enter` - Выбрать и вставить элемент
- `⌘1-⌘9` - Быстрый выбор элемента по номеру
- `Escape` - Закрыть окно

### Системный трей

- Левый клик - показать окно
- Правый клик - меню с опциями
  - Показать историю
  - Очистить историю
  - Настройки
  - Завершить

## Конфигурация

История сохраняется в:
- `~/.local/share/ClipboardManager/clipboard.db`

## Требования

- Qt6 (Core, Gui, Widgets, Sql, DBus)
- CMake 3.20+
- C++20 компилятор
- Linux с KDE/GNOME (для глобальных горячих клавиш)

## Лицензия

MIT License
