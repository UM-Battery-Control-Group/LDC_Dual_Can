%C:\Users\Jason\STM32CubeIDE\workspace_1.3.0\LDCquadCan
%28 TURNS

   LDC1614_resistance = 28.88;
   LDC1614_inductance = 78.3;

   LDC1614_capacitance = 270;

   LDC1614_Q_factor = 52.5;
 LDC1614_Fsensor = 1 / (2 * 3.14 * sqrt(LDC1614_inductance * LDC1614_capacitance * 10^ -18)) *(10^ -6)

 FIN_DIV = (LDC1614_Fsensor / 8.75 + 1);



 %36 TURNS

   LDC1614_resistance = 64.04;
   LDC1614_inductance = 470;

   LDC1614_capacitance = 200;

   LDC1614_Q_factor = 41;
 LDC1614_Fsensor = 1 / (2 * 3.14 * sqrt(LDC1614_inductance * LDC1614_capacitance * 10^ -18)) *(10^ -6)

 FIN_DIV = (LDC1614_Fsensor / 8.75 + 1);