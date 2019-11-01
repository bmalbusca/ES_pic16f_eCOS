# Good morning fellows!
Acho que está tudo menos o SLEEP() e adicionar parâmetros de recuperação.

- As variáveis que correspondiam aos threshold mudei o nome para corresponderem
aos nomes que o stôr deu no projeto: alat, alal, alaf (flag se queremos alarmes)
e deixei o alarm.

- O **configuration mode** foi feito esperando em while() por alterações nos switch,
o select_mode está a ser incrementado na rotina de interrupção do S1.

- Os botões têm o *debounce* feito e estão protegidos das interrupções.
  - O *debounce* está na interrupção de S1 (para S1) e na interrupção do
 timer0 (para S2) que é um relógio ao milisegundo.
  - A proteção contra interrupções é feita através das funções ReadS1() e
 ReadS2(), onde as variáveis que representam os botões premidos s1_pressed
 e s2_pressed também são colocados a zero após leitura.
 
- O PWM não estava com duty_cycle crescente, adicionei também isso; está
numa das interrupções acho.

- Devo ter alterado mais cenas, mas nada de especial.

- Os parâmetros de recuperação começam no endereço EE_RECV + 1.
