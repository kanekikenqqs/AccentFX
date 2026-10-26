# Accent FX — VST3 «акцентная» педаль для djent / metalcore

Мультиэффект, который включается в нужные моменты (брейкдаун, переход, финальный аккорд),
чтобы сделать акцент: октавы, whammy, dive-bomb, глитч, риз­еры, шиммер и т.д.
28 эффектов, XY-пэд, рандомизатор, 16-шаговый trance-gate, bypass.

## Сборка (Windows / macOS / Linux)
Нужны: CMake ≥ 3.22, C++17-компилятор (VS 2022 / Xcode / gcc 11+),
на Linux ещё: `libasound2-dev libfreetype-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libgl1-mesa-dev`.

    cmake -B build -DCMAKE_BUILD_TYPE=Release          # JUCE скачается сам
    cmake --build build --config Release -j
    # или со своим JUCE:  cmake -B build -DJUCE_PATH=C:/dev/JUCE

VST3 копируется автоматически в системную папку плагинов (COPY_PLUGIN_AFTER_BUILD).
Рядом собирается Standalone — его можно использовать как реальную педаль с аудиоинтерфейсом.

DSP-ядро (`Source/Engine.h`) не зависит от JUCE и покрыто тестами:
`g++ -std=c++17 -O2 tests/test_engine.cpp -o t && ./t`

## Управление
* **ACCENT** (большая кнопка справа внизу) — включает эффекты. Режим Latch (щелчок) или Hold (пока нажата);
  режим переключается кликом по подписи под кнопкой. Параметр `Accent` можно автоматизировать / вешать на MIDI-педаль
  в DAW, либо включить **MIDI Hold** (Setup): любая зажатая нота = акцент.
* **BYPASS** (справа вверху) — полный обход. Это настоящий host-bypass (DAW видит его как bypass-параметр),
  переход плавный, в обходе плагин не грузит процессор.
* **XY-пэд**: в 4 углах назначаются любые эффекты (клик по названию слева). Режим Radial: центр = ничего,
  дальше от центра = сильнее, направление выбирает эффект. Режим Morph: плавное смешивание всех четырёх.
  «Touch = Accent» — прикосновение к пэду само включает акцент, «Spring» — пэд возвращается в центр.
* **Randomize** (кнопка с перекрещенными стрелками) + **Chaos**. Таблетки справа задают, какие группы
  участвуют (Pitch, FX, Glitch, Mod, Gen, Time, Pad = углы пэда, Gate = рисунок гейта).
  *Auto Randomize* (Setup): новый случайный патч на каждом включении или выключении акцента.
* **Gate keys** (16 клавиш внизу) — ритм trance-gate; клик/протяжка. Шаг задаётся на странице Rack.
* **Rack** — все 28 эффектов с полным набором ручек. **Setup** — глобальные настройки.

## Эффекты (28)
Pitch: Oct Down, Oct Up, Harmony (интервал), Whammy (позиция), Dive Bomb, Detune
FX: Fuzz, Bit Crush, Ring Mod, Filter (LP/BP/HP + свип), Radio, Auto Wah
Glitch: Stutter, Reverse, Tape Stop, Trance Gate, Freeze, Scatter
Mod: Chorus, Flanger, Phaser, Tremolo (синхр.), Vibrato
Gen: Sub Drop, Riser
Time: Delay (ping-pong, синхр.), Reverb (FDN), Shimmer

Хвосты Delay/Reverb/Shimmer продолжают звучать после выключения акцента.
Stutter, Freeze, Tape Stop, Dive, Sub Drop, Riser срабатывают в момент включения акцента.

## Заметки
* Питч-эффекты — классический двухтактовый delay-line shifter; окно (Window) 20–120 мс — компромисс между
  задержкой и артефактами на низких струнах (для 7–8-струнок ставьте 60–90 мс).
* Синхронизация (Gate, Delay, Tremolo, Stutter, Reverse, Scatter) идёт от темпа/позиции хоста,
  без транспорта — от внутреннего темпа 120 BPM.
* Плагин не вносит латентность (питч-эффекты имеют собственную «мягкую» задержку в мокром сигнале).

## Windows-установщик (AccentFX-Setup.exe)
Вариант 1 (без установки чего-либо у себя): залейте проект в GitHub-репозиторий →
вкладка Actions → «Build Windows installer» → Run workflow → скачайте артефакт `AccentFX-Setup`.
Вариант 2 (локально): установите Visual Studio 2022 (C++), CMake и Inno Setup 6, затем запустите `build_windows.bat`;
результат — `dist\AccentFX-Setup.exe`.
Установщик кладёт VST3 в `C:\Program Files\Common Files\VST3`, Standalone — в Program Files.
