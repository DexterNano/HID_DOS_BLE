# HID_DOS_BLE
Este fichero usa el NRF52 del puck.js para disparar la funcion "locate" originalmente encontrada dentro de HID Reader Manager App

Esta genera una conversacion usando Notify de cambios en el valor de una característica (characteristicUuid).
Luego nosotros escribimos en esa misma variable, dependiendo el valor, hay que responder con uno u otro. Para el reversing he usado el log de Android con la opcion de HCI Snoop (D)Log. Usando el filtro "btatt", luego usando
el repositorio de startrk1995 en github, para poner nombres a los valores y no llamarlos 1,2,3...

ToDo's: 
* Meter alguna forma de atacar a todos los lectores presentes, no de 1 en 1 (cambiar requestdevice por scan NRF)
* Capturar la respuesta a 0E12
* Fix Unhandled promise rejection: InternalError: BLE task completed that wasn't scheduled (SERVICE/NONE) 
* Cambiar el comportameinto del boton, presionado largo VS corto y opcion hail mary 
* Cuando quitas Beep de la transmision, sigue pitando pero se cancela con una tarjeta y parece que no se bloquea,
    quiero ver si esto implica que hay mas formas de modificar el comportamiento del lector, modos de  solo           pitido, solo bloqueo etc...
    


Una ejecucion valida es la siguiente 
WRONG:c00a440aa000440011010            -> Wrong AID1 
WRONG:c00a440aa00038202d0110           -> Wrong AID2 
AID:  c00a440aa00038202f0110           -> Correct AID
ACK:  c00da550e121bd21b062dc81949279   -> ACK
PART1:c00ca0000                        -> PartA
PART2:c00ca0000                        -> PartB
ACK:  c00da730009bd7a45813dc61         -> ACK
BEEP: c00ca0000                        -> BeepModern
Disconnected  22

