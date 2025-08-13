# Rama_2Nodes_Test
* Prueba la capacidad de conexión usando la capa de convergencia Rama dentro del protocolo HDTN.

## Compilación de HDTN
1. `export HDTN_SOURCE_ROOT=/ruta/a/HDTN`
2. `cd $HDTN_SOURCE_ROOT`
3. `mkdir build`
4. `cd build`
5. `cmake ..`
6. `make -j8`
7. `make install`

## Cómo usar
* Dentro del contenedor "receptor", ejecutar `./runscript_receiver_rama.sh`
* Esperar aproximadamente 3 segundos
* Dentro del contenedor "emisor", ejecutar `./runscript_sender_rama.sh`
* Esperar aproximadamente 10 segundos para que los programas se inicialicen y operen
* Puede utilizar `tcpdump` para observar el tráfico de red en cualquiera de los contenedores o en la máquina anfitriona
