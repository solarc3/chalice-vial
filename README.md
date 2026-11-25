# Vial por USB en el Chalice Mugitya 40 BLE original

El Chalice original se programaba con una aplicación de LDN para Windows. Para usar Vial, usamos el perfil de fábrica para reconstruir las conexiones de las teclas y el cargador USB para reemplazar el programa del teclado. Primero hicimos que escribiera con QMK. Después incorporamos Vial y reservamos una parte de la memoria interna para guardar los cambios al desconectar el cable. La versión 0.0.2 cubre USB y las 46 teclas de la capa base. Bluetooth y las funciones Fn de LDN quedan pendientes.

## Cómo se hizo

1. **Identificamos la placa.** La serigrafía dice «Chalice Mugitya» y «Powered By LDN». El microcontrolador es un STM32F411CEU6. Linux mostraba `0483:2020` en funcionamiento normal y `0483:2021` al entrar en el cargador de LDN. Ambos modos usan USB HID. [Foto de la placa](docs/pcb.png) · [Detalle del STM32](docs/stm32.png)
2. **Reconstruimos el teclado a partir del paquete de fábrica.** Del archivo `Chalice_Mugitya_40_BLE.DMPhyCfg` sacamos los pines, la posición eléctrica y el tamaño de cada tecla. Del archivo `function_layer_0.Fl_Cfg` obtuvimos sus códigos USB. El firmware original activa una fila en alto y lee las columnas. El escáner del port reproduce ese comportamiento con las columnas en entrada con pull-down.
3. **Probamos el camino de actualización.** Ejecutamos el configurador de Windows bajo Wine/Proton en Linux. Con el teclado físico conectado, «Refresh list» mostró «Chalice[USB BL]». El selector rechazó el binario crudo de QMK. Analizamos cómo la aplicación lee el contenedor `.Ldn` y escribimos un empaquetador. Primero reprodujimos el archivo de fábrica byte por byte. Después verificamos que al decodificar el nuevo paquete recuperábamos exactamente el binario compilado. [Empaquetador](keyboards/ldn/chalice_mugitya_40_ble/pack_ldn.py)
4. **Llevamos QMK a la placa.** Enlazamos la aplicación en `0x08020000`, la dirección de la aplicación original. La primera versión arrancó desde el cargador LDN y Linux la enumeró como `FEED:C140`. Por VIA leímos las 46 asignaciones, cambiamos temporalmente la flecha derecha a F13 y restauramos su código. Una pulsación física apareció en la fila 3, columna 11. Esta primera versión guardaba los cambios en RAM.
5. **Pasamos a Vial y separamos los ajustes del código.** Compilamos el mismo escáner con Vial-QMK, incorporamos la definición del teclado al firmware y habilitamos la EEPROM emulada en flash. Vial puede pedir al teclado su propia distribución. La combinación de desbloqueo son las teclas físicas de la esquina superior izquierda e inferior derecha, End y flecha derecha en la capa base.

## Pruebas en la unidad

Desde el primer QMK volvimos a entrar al cargador físico de LDN y cargamos Vial 0.0.2 con el mismo actualizador. Tras el reinicio, Linux mostró «Chalice Mugitya 40 USB Vial». El teclado respondió al protocolo Vial 6, devolvió una definición comprimida de 328 bytes que coincidió con el JSON de compilación y entregó las 46 asignaciones iniciales esperadas. La aplicación Vial de escritorio reconoció las cuatro capas sin importar un JSON. [Captura de Vial en esta unidad](docs/vial-escritorio.png)

También comprobamos que los cambios sobreviven al desconectar el USB. Asignamos F13 a la posición de la flecha derecha en la capa 3 y desconectamos y reconectamos el cable. Linux volvió a enumerar el mismo teclado (dispositivo USB 9 → 10). A los 69 segundos de arrancar, el firmware devolvió F13 en esa posición. Restauramos su valor original, transparente (`KC_TRNS`), y lo volvimos a leer para verificarlo. La flecha derecha de la capa base siguió con su asignación habitual.

## Matriz y almacenamiento

| Elemento | Configuración |
| --- | --- |
| Filas, en orden | PA3, PC15, PC14, PC13 |
| Columnas, en orden | PB10, PB2, PB1, PB0, PB12, PB13, PB14, PB15, PA8, PA9, PA15, PB3, PB4 |
| Aplicación | Sector 5 de la flash interna, desde `0x08020000`, limitado a 128 KiB |
| Ajustes de Vial | Sector 7, desde `0x08060000`. EEPROM lógica de 8 KiB con 16 KiB para el registro de escrituras |
| Identificadores | USB `FEED:C140` de desarrollo. UID Vial `B6 BE 0A 91 68 18 37 ED` |

El STM32 borra el sector 7 completo cuando necesita compactar el registro. Antes de reclamarlo por primera vez, el port comprueba que esté vacío y escribe una marca de propiedad al final. En futuros arranques reconoce su propia marca. La memoria SPI NOR externa queda disponible para el sistema de LDN. [Protección del sector](keyboards/ldn/chalice_mugitya_40_ble/settings_sector.c) · [Manual de referencia del STM32F411](https://www.st.com/resource/en/reference_manual/dm00119316.pdf)

### Qué contiene el archivo de actualización

Para esta versión del cargador, el archivo consta de una cabecera de 32 bytes seguida de la aplicación codificada en palabras de 32 bits. Los enteros son little-endian. La cabecera lleva la versión de formato en el desplazamiento 0, la familia del dispositivo en 2, dos marcadores en 4 y 5, el CRC-8 en 6, la versión del paquete en 7 y la longitud de la aplicación en 9. El CRC se calcula sobre el contenido codificado con el polinomio `0x31` y valor inicial cero. El empaquetador comprueba además que el vector de reinicio apunte al área de aplicación del Chalice.

La prueba de referencia usó `ldn_dm_fw_V0.1.6.7.Ldn` del paquete de abril de 2022. Tenía 76.728 bytes y SHA-256 `893b9a837232fafec3e68b264c568a62611a0ce866432622631ab2a4f80b341f`. La coincidencia byte por byte con ese archivo validó el formato. La confirmación de que el código podía ejecutarse llegó después, al reconocer el teclado por USB con el nuevo identificador.

## Reproducir la compilación

Desde la raíz de este proyecto, con Git, Python 3 y el entorno de compilación ARM de QMK instalado, ejecuta lo siguiente. La revisión indicada es la utilizada para este port. [Guía de portabilidad de Vial](https://get.vial.today/docs/porting-to-vial.html)

```sh
git clone https://github.com/vial-kb/vial-qmk.git
git -C vial-qmk checkout dd43959ae5c08d8a28d38a1acf7b04e86b14a344
git -C vial-qmk submodule update --init --recursive
mkdir -p vial-qmk/keyboards/ldn
cp -R keyboards/ldn/chalice_mugitya_40_ble vial-qmk/keyboards/ldn/
cd vial-qmk
make SKIP_GIT=yes ldn/chalice_mugitya_40_ble:vial
python3 keyboards/ldn/chalice_mugitya_40_ble/pack_ldn.py \
  ldn_chalice_mugitya_40_ble_vial.bin Chalice-Vial-USB-0.0.2.Ldn --version 2
```

El paquete incluido mide 42.292 bytes. Su SHA-256 es `4b0c6e5662760fb581874f52b896b676f0a171c82562054817e109bec1cc7b09`. [Paquete de la versión 0.0.2](firmware/Chalice-Vial-USB-0.0.2.Ldn)

## Carga y recuperación

Con la batería apagada y el USB desconectado, mantén pulsada la tecla física de la esquina superior izquierda al conectar el cable. Comprueba que aparezca `0483:2021`. En LDN Dual Mode Keyboard Tools V1.1, conecta con «Chalice[USB BL]», selecciona el paquete, inicia la actualización y confirma el arranque cuando termine la transferencia. Mantén la alimentación hasta que el teclado vuelva a aparecer por USB. Conserva el paquete original de tu unidad para recuperación. [Guía específica del Chalice de LDN](https://blog.csdn.net/lswhome/article/details/123768793) · [Procedimiento de actualización y recuperación del fabricante](https://blog.csdn.net/lswhome/article/details/115428099)
