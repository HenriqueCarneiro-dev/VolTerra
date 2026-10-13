# VolTerra
### Descrição Breve do Jogo (Versão Atualizada)

O *Central de Energia* é um jogo 2D de estratégia e gerenciamento em tempo real no qual o jogador atua como o operador de uma usina elétrica nacional, com foco em agilidade e tomada de decisão:

* *Controle Binário Direto:* Para facilitar a jogabilidade e a programação em C, o painel de controle utiliza chaves simples de *Ligar/Desligar* (enviar ou não enviar energia) para cada região do país.
* *Gestão de Cota e Blackouts:* Com uma produção finita de energia, o jogador precisa equilibrar o fornecimento entre as zonas. Regiões desabastecidas entram em estado de alerta e iniciam uma contagem regressiva para o blackout.
* *Minigames de Eventos (Tratamento de Colisão):* Quando eventos locais ou emergências acontecem (como um show ou queda de energia em prédios governamentais), o jogador precisa resolver minigames interativos — como *guiar um fio por um labirinto de circuitos* ou *conectar cabos desembaralhados*. Essa mecânica atende perfeitamente à exigência de detecção de colisão do edital.
* *Progressão Gradual:* O jogo inicia de forma simples e escala em dificuldade a cada noite, aumentando a quantidade de regiões ativas e a frequência de chamados de emergência.
