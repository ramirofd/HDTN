# Diferencias principales entre HDTN 1.3.x y 2.0.0

## Resumen ejecutivo
- HDTN 2.0.0 introduce una aplicación de transferencia de ficheros punto a punto (`bpfiletransfer`) con telemetría web integrada y soporte de reanudación de fragmentos, inexistente en la rama 1.3.x.【F:common/bpcodec/apps/bpfiletransfer/include/BpFileTransfer.h†L16-L65】【F:common/bpcodec/apps/bpfiletransfer/src/BpFileTransfer.cpp†L114-L222】
- El plano de telemetría adopta una nueva capa de abstracción `WebsocketServer`, reutilizada tanto por la telemetría central como por `bpfiletransfer`, y sustituye la lógica específica de Beast/CivetWeb que se usaba en 1.3.x.【F:module/telem_cmd_interface/include/WebsocketServer.h†L32-L113】【F:module/telem_cmd_interface/src/TelemetryRunner.cpp†L191-L206】
- Se añade el nuevo convergente HILINK (induct/outduct, parámetros de configuración y scripts), ampliando las opciones de transporte frente a 1.3.x.【F:common/induct_manager/include/HilinkInduct.h†L9-L23】【F:common/outduct_manager/include/HilinkOutduct.h†L9-L35】【F:common/config/include/InductsConfig.h†L35-L108】【F:common/config/include/OutductsConfig.h†L35-L113】【F:config_files/hdtn/hdtn_Node1_hilink.json†L1-L62】
- El módulo de CRC añade combinaciones optimizadas y clases para cálculo incremental/out-of-order, junto con pruebas unitarias dedicadas, reforzando la robustez frente a 1.3.x.【F:common/bpcodec/include/codec/Bpv7Crc.h†L23-L92】【F:common/bpcodec/src/codec/Bpv7Crc.cpp†L97-L220】【F:common/bpcodec/src/codec/ZlibCrc32cCombine.cpp†L1-L196】【F:common/bpcodec/test/TestBpv7Crc.cpp†L1-L111】
- Se renuevan los scripts de pruebas: se añaden suites para HILINK, regresión de carreras y streaming multimedia detallado, mientras se retiran los scripts monolíticos antiguos.【F:tests/test_scripts_linux/HILINK_2Nodes_Test/runscript_sender_hilink.sh†L1-L16】【F:tests/test_scripts_linux/Race_Regression/hdtn.json†L1-L62】【F:tests/test_scripts_linux/Streaming/file_streaming/only_video/Streaming_1_Node_Test/runscript_h265_streaming_LTP_one_node.sh†L1-L45】【a6e8a9†L1-L108】
- La canalización CI incorpora el detector de funciones con alta complejidad ciclomática y se simplifican workflows heredados.【F:ci/detect_high_ccm.py†L1-L72】【a6e8a9†L1-L108】

## Aplicaciones y funcionalidades nuevas
### `bpfiletransfer`
La rama 2.0.0 crea la clase `BpFileTransfer`, un `BpSourcePattern` especializado que encapsula:
- Serialización de metadatos de fragmentos, códigos de acuse y reconstrucción de ficheros con control CRC32C end-to-end.【F:common/bpcodec/apps/bpfiletransfer/src/BpFileTransfer.cpp†L59-L214】
- Monitorización de directorios mediante `DirectoryScanner` y exposición de estado vía WebSocket para la GUI incluida en `common/bpcodec/apps/bpfiletransfer/www` (ausente en 1.3.x).【F:common/bpcodec/apps/bpfiletransfer/src/BpFileTransfer.cpp†L185-L343】
- Integración directa con configuraciones de induct/outduct y parámetros BP (custodia, prioridad, caducidad) en el arranque.【F:common/bpcodec/apps/bpfiletransfer/include/BpFileTransfer.h†L52-L59】

### Utilidades de directorio y JSON
- `DirectoryScanner` añade `InterruptTimedWait`, mejorando la respuesta al cierre de hilos que consumen directorios en tiempo de espera; esto es fundamental para la limpieza de `bpfiletransfer` y no existía en 1.3.x.【F:common/util/include/DirectoryScanner.h†L52-L60】【F:common/util/src/DirectoryScanner.cpp†L20-L137】
- Las pruebas de serialización JSON ahora validan claves/valores UTF-8, garantizando compatibilidad internacional (no cubierto antes).【F:common/util/test/TestJsonSerializable.cpp†L19-L55】

## Capa de telemetría y GUI
- Se introduce `WebsocketServer`, una fachada común para Beast y CivetWeb que centraliza las opciones CLI (puerto, raíz del GUI, certificados TLS) y operaciones de envío.【F:module/telem_cmd_interface/include/WebsocketServer.h†L32-L113】
- `TelemetryRunner` y sus opciones pasan a delegar en esta nueva clase; la inicialización ahora falla de forma explícita si el WebSocket no puede arrancar, y las devoluciones usan el mismo canal de abstracción.【F:module/telem_cmd_interface/src/TelemetryRunnerProgramOptions.cpp†L23-L44】【F:module/telem_cmd_interface/src/TelemetryRunner.cpp†L160-L208】
- `bpfiletransfer` reutiliza la misma infraestructura para publicar progreso y recibir comandos sobre WebSocket, unificando la pila GUI.【F:common/bpcodec/apps/bpfiletransfer/src/BpFileTransfer.cpp†L217-L275】

## Convergencias y configuraciones
- El soporte HILINK añade implementaciones dedicadas de induct y outduct que reutilizan las clases UDP pero añaden encabezados y telemetría específicos.【F:common/induct_manager/include/HilinkInduct.h†L9-L23】【F:common/outduct_manager/include/HilinkOutduct.h†L9-L35】
- Las estructuras de configuración incorporan el campo `hilinkHeaderByte`, se amplía la validación JSON y se permite controlar la precisión de rate limiting para enlaces HILINK, algo no contemplado en 1.3.x.【F:common/config/include/InductsConfig.h†L35-L108】【F:common/config/include/OutductsConfig.h†L35-L113】
- Existen nuevas plantillas de configuración (`config_files/hdtn`, `inducts`, `outducts`) para redes HILINK multi-nodo.【F:config_files/hdtn/hdtn_Node1_hilink.json†L1-L62】

## Capa de codificación BP y CRC
- `Bpv7Crc` integra clases orientadas a estado (`Crc32c_InOrderChunks`, `Crc32c_ReceiveOutOfOrderChunks`) capaces de combinar fragmentos recibidos fuera de orden, habilitando verificaciones eficientes para la nueva aplicación de ficheros.【F:common/bpcodec/include/codec/Bpv7Crc.h†L37-L92】【F:common/bpcodec/src/codec/Bpv7Crc.cpp†L178-L220】
- Se incorpora una versión optimizada del algoritmo `crc32_combine` basada en tablas LUT derivadas de zlib, lo cual reduce tiempos de recombinación de CRC en escenarios de fragmentación.【F:common/bpcodec/src/codec/ZlibCrc32cCombine.cpp†L1-L196】
- Nuevas pruebas unitarias validan los CRCs frente a vectores conocidos, fragmentación y ordenación arbitraria, elevando la cobertura frente a 1.3.x.【F:common/bpcodec/test/TestBpv7Crc.cpp†L23-L111】

## Scripts de prueba y escenarios
- Los scripts `HILINK_2Nodes_Test` ejercitan configuraciones específicas del nuevo convergente, incluyendo los archivos JSON con `hilinkHeaderByte` para cada extremo.【F:tests/test_scripts_linux/HILINK_2Nodes_Test/runscript_sender_hilink.sh†L3-L16】
- `Race_Regression` añade un escenario de estrés con STCP/UDP para detectar condiciones de carrera enrutadas, incluyendo configuraciones dedicadas.【F:tests/test_scripts_linux/Race_Regression/hdtn.json†L1-L62】
- La matriz de streaming se reorganiza en subdirectorios (solo vídeo, vídeo+audio) con pipelines GStreamer explícitos para H.264/H.265 y diferentes CLA, aportando granularidad de pruebas frente al set monolítico de 1.3.x.【F:tests/test_scripts_linux/Streaming/file_streaming/only_video/Streaming_1_Node_Test/runscript_h265_streaming_LTP_one_node.sh†L1-L45】【a6e8a9†L1-L108】

## Canalización de integración continua
- Se elimina la duplicidad de workflows GitHub heredados y se introduce `detect_high_ccm.py`, que utiliza Lizard para bloquear funciones nuevas con complejidad >15 al comparar contra la rama objetivo del MR.【F:ci/detect_high_ccm.py†L1-L72】【a6e8a9†L1-L108】

## Cambios de parámetros y limpieza
- Archivos de configuración se reorganizan: ejemplos LTP existentes se mueven de `tests` a `config_files/hdtn`, mientras que varios JSON y scripts de streaming LTP mono-nodo se eliminan (marcados en la estadística de diff).【a6e8a9†L46-L108】
- El script `kill.sh` admite parámetros adicionales para los nuevos procesos HILINK, y se añaden asistentes (`runscript_*`) para escenarios multimedia detallados.【a6e8a9†L1-L108】【F:tests/test_scripts_linux/Streaming/file_streaming/only_video/Streaming_1_Node_Test/runscript_h265_streaming_LTP_one_node.sh†L1-L45】

## Impacto esperado
- **Rendimiento y estabilidad:** las mejoras de CRC y el detector de complejidad ayudan a prevenir regresiones en nuevos módulos de alto tráfico como `bpfiletransfer`.
- **Operaciones y despliegue:** la abstracción `WebsocketServer` y la reorganización de scripts facilitan la administración de escenarios heterogéneos, simplificando la adopción de 2.0.0.
- **Compatibilidad:** aunque la mayoría de configuraciones previas siguen válidas, los nuevos campos (`hilinkHeaderByte`) y rutas de scripts implican actualizar automatizaciones al migrar desde 1.3.x.
