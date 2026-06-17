// FrogEnsemble.pde (Processing側：キーボード入力テストおよびBPM可視化)

import ddf.minim.*;
import ddf.minim.ugens.*;
import processing.serial.*;

Minim minim;
AudioOutput out;

Oscil tOsc1, tOsc2, tOsc3, tOsc4, tOsc5;
Oscil oOsc1, oOsc2, oOsc3;

Serial myPort; 
float serialFreq = 0.0f;
float serialTrumpetAmp = 0.0f;
float serialOrganAmp = 0.0f;
int serialBpm = 90; // Arduinoから受け取る現在のBPM

boolean isPlaying = false;

void setup() {
  size(500, 320); 
  pixelDensity(1); 
  
  printArray(Serial.list());
  String portName = Serial.list()[2]; // 環境に合わせてインデックスを変更してください
  myPort = new Serial(this, portName, 115200);
  myPort.bufferUntil('\n'); 
  
  minim = new Minim(this);
  out = minim.getLineOut(Minim.STEREO, 1024);
  
  tOsc1 = new Oscil(0, 0f, Waves.SINE); tOsc1.patch(out);
  tOsc2 = new Oscil(0, 0f, Waves.SINE); tOsc2.patch(out);
  tOsc3 = new Oscil(0, 0f, Waves.SINE); tOsc3.patch(out);
  tOsc4 = new Oscil(0, 0f, Waves.SINE); tOsc4.patch(out);
  tOsc5 = new Oscil(0, 0f, Waves.SINE); tOsc5.patch(out);
  
  oOsc1 = new Oscil(0, 0f, Waves.SINE); oOsc1.patch(out);
  oOsc2 = new Oscil(0, 0f, Waves.SINE); oOsc2.patch(out);
  oOsc3 = new Oscil(0, 0f, Waves.SINE); oOsc3.patch(out);
}

void draw() {
  background(30);
  fill(255);
  textSize(16);
  
  if (serialFreq > 0) {
    tOsc1.setFrequency(serialFreq * 1.0f);
    tOsc2.setFrequency(serialFreq * 2.0f);
    tOsc3.setFrequency(serialFreq * 3.0f);
    tOsc4.setFrequency(serialFreq * 4.0f);
    tOsc5.setFrequency(serialFreq * 5.0f);
    
    oOsc1.setFrequency(serialFreq * 1.0f);
    oOsc2.setFrequency(serialFreq * 0.5f);
    oOsc3.setFrequency(serialFreq * 2.0f);
  }
  
  tOsc1.setAmplitude(serialTrumpetAmp * 0.40f);
  tOsc2.setAmplitude(serialTrumpetAmp * 0.25f);
  tOsc3.setAmplitude(serialTrumpetAmp * 0.15f);
  tOsc4.setAmplitude(serialTrumpetAmp * 0.10f);
  tOsc5.setAmplitude(serialTrumpetAmp * 0.05f);
  
  oOsc1.setAmplitude(serialOrganAmp * 1.00f);
  oOsc2.setAmplitude(serialOrganAmp * 0.70f);
  oOsc3.setAmplitude(serialOrganAmp * 0.40f);

  // テキスト情報の描画
  if (!isPlaying) {
    text("Status: Waiting...", 20, 35);
    text("Action: Click screen to START", 20, 65);
  } else {
    text("Status: Playing (Tempo Control Enabled)", 20, 35);
    text("Current Freq: " + serialFreq + " Hz", 20, 65);
  }
  
  // 現在のBPMを画面右上に強調表示
  fill(0, 255, 255);
  text("BPM: " + serialBpm, 380, 35);
  
  textSize(12);
  fill(180);
  text("Test Key: [1]=BPM 70(Slow)  [2]=BPM 90(Mid)  [3]=BPM 110(Fast)", 20, 95);
  
  // リアルタイム波形表示
  stroke(0, 255, 0); 
  strokeWeight(2);
  for (int i = 0; i < out.bufferSize() - 1; i++) {
    float x1 = map(i, 0, out.bufferSize(), 0, width);
    float x2 = map(i+1, 0, out.bufferSize(), 0, width);
    float y1 = 220 + out.left.get(i) * 50;
    float y2 = 220 + out.left.get(i+1) * 50;
    line(x1, y1, x2, y2);
  }
}

void mousePressed() {
  myPort.write('S');
  isPlaying = true;
}

// ★テスト用：キーボード入力を検知し，同期側を模した信号をArduinoへ転送
void keyPressed() {
  if (key == '1' || key == '2' || key == '3') {
    myPort.write(key); // キー情報をそのままArduinoに送りつける
  }
}

void serialEvent(Serial p) {
  String inString = p.readStringUntil('\n');
  if (inString != null) {
    inString = trim(inString);
    String[] list = split(inString, ',');
    // データが正しく4つ（周波数、トランペット音量、オルガン音量、BPM）届いているか確認
    if (list.length == 4) {
      serialFreq = float(list[0]);
      serialTrumpetAmp = float(list[1]);
      serialOrganAmp = float(list[2]);
      serialBpm = int(list[3]);
    }
  }
}

void stop() {
  out.close();
  minim.stop();
  super.stop();
}
