# Contador de Colonias

Software de visión artificial para el conteo de colonias en placas de Petri, desarrollado para el Laboratorio de Microbiología y Biotecnología de Alimentos (MIBIAL) de la Facultad de Ciencias de la Alimentación, UNER.

Detecta y cuenta por separado las colonias violetas y azules (por ejemplo coliformes y *E. coli* en placas cromogénicas), con opción de dividir la placa en dos mitades.

## Descarga (Windows)

Bajar `InstaladorContadorColonias-<versión>.exe` desde la última versión en [Releases](https://github.com/Tmas-mnslla-pnzzo/contador-colonias/releases/latest) y ejecutarlo.

## Compilar en Linux

```
sudo apt install qt6-base-dev libopencv-dev cmake build-essential
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux
./build-linux/ContadorColonias
```

## Nueva versión

1. Cambiar `VERSION` en `CMakeLists.txt`.
2. Commit y tag: `git tag v<versión> && git push && git push --tags`.
3. GitHub Actions compila en Windows, arma el instalador y lo publica en Releases.

Ícono: [OpenMoji](https://openmoji.org), licencia CC BY-SA 4.0.
