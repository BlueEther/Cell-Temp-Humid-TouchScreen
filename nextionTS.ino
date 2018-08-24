void parseNexMsg(String msg){
  if (msg == "65 0 1 0 ff ff ff") {       //Main page button pressed
    nexPage = 1;
    nexButton = 3;
    nexChange = TRUE;
    ckAVgTemp(hourAvg[0][minute()]);
    ckAVgTemp(hourAvg[1][minute()]);
  }
  else if (msg == "65 0 3 0 ff ff ff") { //Graph page button pressed
    nexPage = 3;
    nexButton = 0;
    nexChange = TRUE;
  }
  else if (msg == "65 1 b 1 ff ff ff") { //humidity button on main
    nexPage = 1;
    nexButton = 2;
    nexChange = TRUE;
  }
  else if (msg == "65 1 c 1 ff ff ff") { //top temp button on main
    nexPage = 1;
    nexButton = 3;
    nexChange = TRUE;
  }
  else if (msg == "65 1 e 1 ff ff ff") { //bottom button on main
    nexPage = 1;
    nexButton = 5;
    nexChange = TRUE;
  }
  else if (msg == "65 1 15 0 ff ff ff") {  //save button on main
    nexPage = 1;
    nexButton = 7;
    nexChange = TRUE;
  }
  else if (msg == "65 3 2 0 ff ff ff" || msg == "65 1 14 0 ff ff ff" || msg == "65 2 1 0 ff ff ff") {  //menu button 
    nexPage = 0;
    nexButton = 0;
    nexChange = false;
  }
  else if (msg == "65 3 4 1 ff ff ff") {  //top temp button on gauge
    nexPage = 3;
    nexButton = 0;
    nexChange = TRUE;
  }
  else if (msg == "65 3 5 1 ff ff ff") {  //top temp button on gauge
    nexPage = 3;
    nexButton = 1;
    nexChange = TRUE;
  }
  else if (msg == "65 3 6 1 ff ff ff") {  //top temp button on gauge
    nexPage = 3;
    nexButton = 2;
    nexChange = TRUE;
  }
}
