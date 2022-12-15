H HVM
página HHVM | Documentación de HHVM | Página de hacklang | Grupo general | Grupo de desarrollo | Gorjeo

HHVM es una máquina virtual de código abierto diseñada para ejecutar programas escritos en Hack . HHVM utiliza un enfoque de compilación justo a tiempo (JIT) para lograr un rendimiento superior al mismo tiempo que mantiene una increíble flexibilidad de desarrollo.

HHVM debe usarse junto con un servidor web como el Proxygen integrado y fácil de implementar , o un servidor web basado en FastCGI sobre nginx o Apache.

Instalando
Si es nuevo, pruebe nuestra guía de inicio .

Puede instalar un paquete precompilado o compilar desde el código fuente .

Correr
Puede ejecutar programas independientes simplemente pasándolos a hhvm: hhvm example.hack.

Si desea alojar un sitio web:

Instala tu servidor web favorito. Proxygen está integrado en HHVM, rápido y fácil de implementar.
Instala nuestro paquete
Inicie su servidor web
Corrersudo /etc/init.d/hhvm start
Visite su sitio enhttp://.../main.hack
Nuestra guía de introducción proporciona una introducción un poco más detallada, así como enlaces a más información.

contribuyendo
Nos encantaría contar con su ayuda para mejorar HHVM. Si está interesado, lea nuestra guía para contribuir .

Licencia
HHVM tiene licencia de PHP y Zend, excepto que se indique lo contrario.

El verificador de tipos Hack tiene la licencia MIT , excepto que se indique lo contrario.

La biblioteca estándar de Hack tiene la licencia MIT , excepto que se indique lo contrario.

Informes de bloqueos
Consulte Informar bloqueos para obtener consejos útiles sobre cómo informar bloqueos de una manera procesable.

Seguridad
Para obtener información sobre cómo informar vulnerabilidades de seguridad en HHVM, consulte SECURITY.md .

Preguntas más frecuentes
Nuestras preguntas frecuentes para usuarios tienen respuestas a muchas preguntas comunes sobre HHVM, desde preguntas generales hasta preguntas dirigidas a aquellos que quieren usar .

También hay preguntas frecuentes para los colaboradores de HHVM.

Lanzamientos 15
HHVM-3.15.0
Más reciente
el 28 de septiembre de 2016
