# Chalice Mugitya 40 BLE de LDN

Port USB para el modelo original con STM32F411CEU6. El mapa Vial incorpora 46 teclas y cuatro capas. Para desbloquear los ajustes protegidos, sigue el diálogo de Vial y mantén pulsadas las esquinas superior izquierda e inferior derecha del teclado. El desbloqueo usa posiciones físicas y sigue funcionando al reasignar esas teclas.

Desde la raíz de Vial-QMK, compila con `make ldn/chalice_mugitya_40_ble:vial`. El cargador original usa paquetes `.Ldn`; el script `pack_ldn.py` recibe el binario de aplicación enlazado en `0x08020000`. Los ajustes se guardan mediante EEPROM emulada en el sector 7 de la flash interna.
