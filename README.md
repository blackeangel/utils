# utils

**[English](#english) | [Русский](#русский)**

---

<a name="english"></a>
## English

A collection of command-line utilities for working with Android firmware images, built as a single binary for Android (ARM/ARM64) and Linux.

### Usage

```
utils <function> [options...]
utils help                  — show full help
utils help <function>       — show help for a specific function
```

### Functions

#### `block_finder`
Detect shared blocks in sparse ext4 / raw ext4 images.
```
utils block_finder [output_path_file]
```
`output_path_file` — optional path to save results (default: `/sdcard/block_info.txt`)

---

#### `chunk_split`
Split a raw image into multiple sparse chunks.
```
utils chunk_split [-s suffix] [-B block_size] [-C chunk_size] [-P parts_count] <input_file> [output_dir]
```

---

#### `copy`
Copy a region of a file to a new file.
```
utils copy <source_file> -d|-h|-o <start> -d|-h|-o <length_or_end> [<output_file>]
```
- `-d` — value in decimal
- `-h` — value in hexadecimal
- `-o` — value as absolute offset (end position)

---

#### `cut`
Same as `copy` but removes the region from the source file.
```
utils cut <source_file> -d|-h|-o <start> -d|-h|-o <length_or_end> [<output_file>]
```

---

#### `delgaaps`
Remove GApps and other packages from an unpacked system image.
```
utils delgaaps <folder> <file_list>
```
- `<folder>` — path to the unpacked image directory
- `<file_list>` — text file with a list of paths to remove

---

#### `file_explorer`
Browse files and directories with optional filters.
```
utils file_explorer [directory] [options] [filters]
```
- `-d` — show directories only
- `-f` — show files only (applies filters: `.img .dat .br .list` by default)

---

#### `foffset`
Find a hex pattern in a file and return its offset(s).
```
utils foffset <file> <hexstring> [options...]
```
Options: `-s <offset>` `-l <length>` `-n <count>` `-b <bufKB>` `-r` `-d` `-hu` `-hp` `-hup` `-o` `-q`

Hex pairs can be separated by space, `.`, `,`, `-` or `:` — e.g. `41.35:AA.55`

---

#### `fstab_fix`
Fix `fstab` files: remove `avb`/`avb2`/`verify` flags, add `erofs` entries, optionally replace `ro` with `rw`.
```
utils fstab_fix [-rw] [-b | -br /path/to/backup] <folder_or_file> [...]
```
- `-rw` — replace `ro` → `rw` in ext4 lines
- `-b <dir>` — backup fstab files to directory
- `-br <dir>` — backup and overwrite if backup already exists

---

#### `hexpatch`
Find and replace a hex pattern in a file.
```
utils hexpatch <file> <find_hex> <replace_hex> <way>
```
- `way = 0` — first match from start (default)
- `way = 1` — all matches
- `way = -1` — first match from end

---

#### `insert`
Insert a file into another file at a given offset.
```
utils insert <file> <offset> <insert_file>
```

---

#### `kerver`
Extract and display kernel version information from a kernel image (supports gzip-compressed kernels).
```
utils kerver <kernel_file>
```

---

#### `logo_mtk`
Unpack or repack the `logo.bin` file of MediaTek devices.
```
utils logo_mtk unpack <logo.bin> <output_folder>
utils logo_mtk pack   <input_folder> <output_logo.bin>
```

---

#### `md1img`
Unpack or repack MediaTek modem images (`md1img.img`, `modemd.img`).  
Supports gz, XZ container, and legacy LZMA raw stream — compression parameters are auto-detected and preserved on repack.
```
utils md1img unpack <md1img.img> [output_dir]
utils md1img pack   <input_dir>  [output_file]
```
If `output_dir` / `output_file` is omitted, defaults are placed next to the input.

---

#### `sdat2img`
Convert a sparse `.dat` + transfer list to a raw image.
```
utils sdat2img <transfer_list> <new_dat_file> <output_img_file>
```

---

#### `shared_block_detector`
Detect shared blocks in sparse ext4 and raw ext4 files.
```
utils shared_block_detector <file>
```

---

#### `writekey`
Write a key value into a binary image at a given offset.
```
utils writekey <image_file> <offset> <-f|-h|-b> <value>
```
- `-f` — value from file
- `-h` — value as hex string
- `-b` — create backup before writing

---

#### `avb_fixed`
Parse AVB/vbmeta image info and optionally disable AVB verification flags.  
Output format matches `avbtool info_image`.
```
utils avb_fixed <vbmeta.img>
utils avb_fixed <vbmeta.img> --disable_verified
utils avb_fixed <vbmeta.img> --restore
```
- *(no flag)* — print full AVB info (algorithm, public key SHA-1, all descriptors, current flags)
- `--disable_verified` — set `HASHTREE_DISABLED` and/or `VERIFICATION_DISABLED` flags in the image
- `--restore` — clear both flags

> ⚠️ Patching invalidates the RSA signature. The device bootloader must be unlocked to boot a patched vbmeta.

Patch methods selected automatically:
- **Method 1** — set `HASHTREE_DISABLED` only (verification was already off)
- **Method 2** — set `VERIFICATION_DISABLED` only (hashtree was already off)
- **Method 3** — set both flags (device fully locked)

---

#### `append2simg`
Append a raw image to an existing sparse image.
```
utils append2simg <output_sparse.img> <input_raw.img>
```

---

#### `img2simg`
Convert a raw image to sparse format.
```
utils img2simg [-s] <raw_image> <sparse_image> [block_size]
```
- `-s` — hole mode: detect and skip zero regions
- `block_size` — default 4096, must be a multiple of 4 and ≥ 1024

---

#### `simg2img`
Convert one or more sparse images to a single raw image.
```
utils simg2img <sparse1> [sparse2 ...] <raw_output>
```

---

#### `simg2simg`
Split a sparse image into multiple smaller sparse images.
```
utils simg2simg <input_sparse> <output_prefix> <max_size_bytes>
```
Output files: `<prefix>.0`, `<prefix>.1`, …

---

### Building

The project supports two build methods: **ndk-build** (via `Android.mk`) and **CMake + Ninja** (via `CMakeLists.txt`).  
CMake is preferred — it automatically downloads `liblzma` (XZ Utils) via `FetchContent`.

#### Requirements

| Tool | Version |
|------|---------|
| Android NDK | r21+ (r25+ recommended) |
| CMake | 3.20+ |
| Ninja | any recent |
| C++ compiler | C++20 support |

---

#### Method 1 — CMake + Ninja (Linux / Termux / macOS)

```bash
# Clone
git clone https://github.com/blackeangel/utils.git
cd utils

# Set NDK path
export ANDROID_NDK=/path/to/android-ndk

# Build for arm64-v8a
mkdir build && cd build
cmake .. \
  -DANDROID_ABI=arm64-v8a \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_NATIVE_API_LEVEL=30 \
  -GNinja
ninja

# Binary: build/utils   (or out/arm64-v8a/bin_utils after build2.sh)
```

To build all ABIs at once, use the provided script:
```bash
chmod +x build2.sh
./build2.sh
# Outputs: out/armeabi-v7a/bin_utils  out/arm64-v8a/bin_utils
```

> **Note:** CMake automatically fetches `liblzma` (XZ Utils) from GitHub on first build.  
> Internet access is required, or pre-run `cmake ..` once with internet and then build offline.

---

#### Method 2 — CMake + Ninja (Windows)

```bat
set ANDROID_NDK=C:\android-ndk

mkdir build && cd build
cmake.exe .. ^
  -DANDROID_ABI=arm64-v8a ^
  -DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK%\build\cmake\android.toolchain.cmake ^
  -DANDROID_NATIVE_API_LEVEL=30 ^
  -GNinja
ninja.exe
```

Or use the script (edit NDK path inside first):
```bat
build2.bat
```

---

#### Method 3 — ndk-build (Linux / Termux)

```bash
export PATH=$PATH:/path/to/android-ndk
ndk-build NDK_PROJECT_PATH=. NDK_APP_LIBS_OUT=out NDK_APPLICATION_MK=Application.mk
# Outputs: out/armeabi-v7a/bin_utils  out/arm64-v8a/bin_utils
```

Or use the script:
```bash
chmod +x build.sh
./build.sh
```

---

#### Method 4 — ndk-build (Windows)

```bat
set PATH=%PATH%;C:\android-ndk
ndk-build NDK_PROJECT_PATH=. NDK_APP_LIBS_OUT=out NDK_APPLICATION_MK=Application.mk
```

Or use the script (edit NDK path inside first):
```bat
build.bat
```

---

#### Native build on Linux (for testing, not Android)

```bash
mkdir build_native && cd build_native
cmake .. -GNinja
ninja
./utils help
```

---

<a name="русский"></a>
## Русский

Набор утилит командной строки для работы с образами прошивок Android, собранных в один бинарный файл для Android (ARM/ARM64) и Linux.

### Использование

```
utils <функция> [параметры...]
utils help                  — показать полную справку
utils help <функция>        — показать справку по конкретной функции
```

### Функции

#### `block_finder`
Обнаружение shared-блоков в sparse ext4 / raw ext4 образах.
```
utils block_finder [output_path_file]
```
`output_path_file` — необязательный путь к файлу с результатами (по умолчанию: `/sdcard/block_info.txt`)

---

#### `chunk_split`
Разбить raw-образ на несколько sparse-чанков.
```
utils chunk_split [-s suffix] [-B block_size] [-C chunk_size] [-P parts_count] <input_file> [output_dir]
```

---

#### `copy`
Скопировать регион файла в новый файл.
```
utils copy <source_file> -d|-h|-o <start> -d|-h|-o <length_or_end> [<output_file>]
```
- `-d` — значение в десятичном формате
- `-h` — значение в шестнадцатеричном формате
- `-o` — значение как абсолютное смещение конца

---

#### `cut`
То же, что `copy`, но вырезает регион из исходного файла.
```
utils cut <source_file> -d|-h|-o <start> -d|-h|-o <length_or_end> [<output_file>]
```

---

#### `delgaaps`
Удалить GApps и другие пакеты из распакованного системного образа.
```
utils delgaaps <folder> <file_list>
```
- `<folder>` — путь к директории с распакованным образом
- `<file_list>` — текстовый файл со списком путей для удаления

---

#### `file_explorer`
Просмотр файлов и директорий с фильтрацией.
```
utils file_explorer [directory] [options] [filters]
```
- `-d` — только директории
- `-f` — только файлы (применяются фильтры: `.img .dat .br .list` по умолчанию)

---

#### `foffset`
Поиск hex-паттерна в файле с выводом смещений.
```
utils foffset <file> <hexstring> [options...]
```
Опции: `-s <offset>` `-l <length>` `-n <count>` `-b <bufKB>` `-r` `-d` `-hu` `-hp` `-hup` `-o` `-q`

Байты hex-строки можно разделять пробелом, `.`, `,`, `-` или `:` — например: `41.35:AA.55`

---

#### `fstab_fix`
Исправление `fstab` файлов: удаление флагов `avb`/`avb2`/`verify`, добавление записей для `erofs`, опционально замена `ro` на `rw`.
```
utils fstab_fix [-rw] [-b | -br /path/to/backup] <folder_or_file> [...]
```
- `-rw` — заменить `ro` → `rw` в строках ext4
- `-b <dir>` — создать резервную копию fstab-файлов в директорию
- `-br <dir>` — резервная копия с перезаписью, если уже существует

---

#### `hexpatch`
Найти и заменить hex-паттерн в файле.
```
utils hexpatch <file> <find_hex> <replace_hex> <way>
```
- `way = 0` — первое совпадение с начала файла (по умолчанию)
- `way = 1` — все совпадения
- `way = -1` — первое совпадение с конца файла

---

#### `insert`
Вставить файл в другой файл по заданному смещению.
```
utils insert <file> <offset> <insert_file>
```

---

#### `kerver`
Извлечь и показать информацию о версии ядра из образа (поддерживаются gzip-сжатые ядра).
```
utils kerver <kernel_file>
```

---

#### `logo_mtk`
Распаковать или запаковать файл `logo.bin` устройств MediaTek.
```
utils logo_mtk unpack <logo.bin> <output_folder>
utils logo_mtk pack   <input_folder> <output_logo.bin>
```

---

#### `md1img`
Распаковать или запаковать образы модема MediaTek (`md1img.img`, `modemd.img`).  
Поддерживаются форматы gz, XZ-контейнер и legacy LZMA raw stream — параметры сжатия определяются автоматически и сохраняются при переупаковке.
```
utils md1img unpack <md1img.img> [output_dir]
utils md1img pack   <input_dir>  [output_file]
```
Если `output_dir` / `output_file` не указан, результат помещается рядом с входным файлом.

---

#### `sdat2img`
Конвертировать sparse `.dat` + transfer list в raw-образ.
```
utils sdat2img <transfer_list> <new_dat_file> <output_img_file>
```

---

#### `shared_block_detector`
Обнаружение shared-блоков в sparse ext4 и raw ext4 файлах.
```
utils shared_block_detector <file>
```

---

#### `writekey`
Записать значение ключа в бинарный образ по заданному смещению.
```
utils writekey <image_file> <offset> <-f|-h|-b> <value>
```
- `-f` — значение из файла
- `-h` — значение как hex-строка
- `-b` — создать резервную копию перед записью

---

#### `avb_fixed`
Парсинг AVB/vbmeta образа и опциональное отключение флагов верификации AVB.  
Формат вывода соответствует `avbtool info_image`.
```
utils avb_fixed <vbmeta.img>
utils avb_fixed <vbmeta.img> --disable_verified
utils avb_fixed <vbmeta.img> --restore
```
- *(без флага)* — вывести полную информацию AVB (алгоритм, SHA-1 публичного ключа, все дескрипторы, текущие флаги)
- `--disable_verified` — установить флаги `HASHTREE_DISABLED` и/или `VERIFICATION_DISABLED`
- `--restore` — сбросить оба флага

> ⚠️ Патчинг делает RSA-подпись невалидной. Загрузчик устройства должен быть разблокирован для загрузки с патченым vbmeta.

Методы патчинга выбираются автоматически:
- **Метод 1** — только `HASHTREE_DISABLED` (верификация уже отключена)
- **Метод 2** — только `VERIFICATION_DISABLED` (hashtree уже отключён)
- **Метод 3** — оба флага (устройство полностью заблокировано)

---

#### `append2simg`
Добавить raw-образ к существующему sparse-образу.
```
utils append2simg <output_sparse.img> <input_raw.img>
```

---

#### `img2simg`
Конвертировать raw-образ в sparse-формат.
```
utils img2simg [-s] <raw_image> <sparse_image> [block_size]
```
- `-s` — hole mode: определять и пропускать нулевые регионы
- `block_size` — по умолчанию 4096, должен быть кратен 4 и ≥ 1024

---

#### `simg2img`
Конвертировать один или несколько sparse-образов в единый raw-образ.
```
utils simg2img <sparse1> [sparse2 ...] <raw_output>
```

---

#### `simg2simg`
Разбить sparse-образ на несколько меньших sparse-образов.
```
utils simg2simg <input_sparse> <output_prefix> <max_size_bytes>
```
Результирующие файлы: `<prefix>.0`, `<prefix>.1`, …

---

### Сборка

Проект поддерживает два способа сборки: **ndk-build** (через `Android.mk`) и **CMake + Ninja** (через `CMakeLists.txt`).  
CMake предпочтителен — он автоматически загружает `liblzma` (XZ Utils) через `FetchContent`.

#### Требования

| Инструмент | Версия |
|------------|--------|
| Android NDK | r21+ (рекомендуется r25+) |
| CMake | 3.20+ |
| Ninja | любая свежая |
| Компилятор C++ | поддержка C++20 |

---

#### Способ 1 — CMake + Ninja (Linux / Termux / macOS)

```bash
# Клонировать репозиторий
git clone https://github.com/blackeangel/utils.git
cd utils

# Указать путь к NDK
export ANDROID_NDK=/path/to/android-ndk

# Сборка для arm64-v8a
mkdir build && cd build
cmake .. \
  -DANDROID_ABI=arm64-v8a \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_NATIVE_API_LEVEL=30 \
  -GNinja
ninja

# Бинарник: build/utils   (или out/arm64-v8a/bin_utils после build2.sh)
```

Для сборки всех ABI сразу используйте готовый скрипт:
```bash
chmod +x build2.sh
./build2.sh
# Результат: out/armeabi-v7a/bin_utils  out/arm64-v8a/bin_utils
```

> **Примечание:** CMake автоматически загружает `liblzma` (XZ Utils) с GitHub при первой сборке.  
> Требуется доступ в интернет, либо выполните `cmake ..` один раз с интернетом, а затем собирайте оффлайн.

---

#### Способ 2 — CMake + Ninja (Windows)

```bat
set ANDROID_NDK=C:\android-ndk

mkdir build && cd build
cmake.exe .. ^
  -DANDROID_ABI=arm64-v8a ^
  -DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK%\build\cmake\android.toolchain.cmake ^
  -DANDROID_NATIVE_API_LEVEL=30 ^
  -GNinja
ninja.exe
```

Или используйте скрипт (предварительно отредактируйте путь к NDK):
```bat
build2.bat
```

---

#### Способ 3 — ndk-build (Linux / Termux)

```bash
export PATH=$PATH:/path/to/android-ndk
ndk-build NDK_PROJECT_PATH=. NDK_APP_LIBS_OUT=out NDK_APPLICATION_MK=Application.mk
# Результат: out/armeabi-v7a/bin_utils  out/arm64-v8a/bin_utils
```

Или через скрипт:
```bash
chmod +x build.sh
./build.sh
```

---

#### Способ 4 — ndk-build (Windows)

```bat
set PATH=%PATH%;C:\android-ndk
ndk-build NDK_PROJECT_PATH=. NDK_APP_LIBS_OUT=out NDK_APPLICATION_MK=Application.mk
```

Или через скрипт (отредактируйте путь к NDK):
```bat
build.bat
```

---

#### Нативная сборка на Linux (для тестирования, не для Android)

```bash
mkdir build_native && cd build_native
cmake .. -GNinja
ninja
./utils help
```
