# Vistrix OS X10 Desktop Edition

Kernel sperimentale x86 a 32 bit avviato da GRUB Multiboot 1.

Componenti:
- richiesta framebuffer 800x600x32
- desktop grafico essenziale stile classico
- barra applicazioni e finestra dimostrativa
- cursore grafico e polling mouse PS/2
- output seriale per diagnostica
- ISO avviabile con GRUB

Build:
    make

Controllo:
    make check

Test headless:
    make run

Il test headless non mostra il framebuffer. Mouse e desktop sono prototipi:
non sono ancora un ambiente completo con interrupt, multitasking,
filesystem, USB o applicazioni.
