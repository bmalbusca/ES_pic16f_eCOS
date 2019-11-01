# Good morning fellows!
Acho que está tudo menos o SLEEP().
A EEPROM também não me inspira confiança, mas parece funcionar xd.

Finalmente percebi a tua ideia Bruno,
com o switch é possível continuar a fazer outras operações no main
enquanto nenhum dos campos foi efetivamente selecionado,
no entanto (por preguiça de pensar), dentro de cada case, após lá dentro
só há uma saída e é clicar no S1 até mais não xd.

- Os botões têm o *debounce* feito e estão protegidos das interrupções.
 - O *debounce* está na interrupção de S1 (para S1) e na interrupção do
 timer0 (para S2) que é um relógio ao milisegundo.
 - A proteção contra interrupções é feita através das funções ReadS1() e
 ReadS2(), onde as variáveis que representam os botões premidos s1_pressed
 e s2_pressed também são colocados a zero após leitura.
 
- O PWM não estava com duty_cycle crescente, adicionei também isso; está
numa das interrupções acho.

- Devo ter alterado mais cenas, mas nada de especial.
