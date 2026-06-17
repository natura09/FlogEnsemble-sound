// Melody.ino (Arduinoメインスケッチ - 演奏途中BPM変更対応)

const float NOTE_C4 = 261.63; // ド
const float NOTE_D4 = 293.66; // レ
const float NOTE_E4 = 329.63; // ミ
const float NOTE_F4 = 349.23; // ファ
const float NOTE_G4 = 392.00; // ソ
const float NOTE_A4 = 440.00; // ラ
const float REST    = 0.0;    // 休符

const float melody[] = {
  NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_E4, NOTE_D4, NOTE_C4, REST, 
  NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_F4, NOTE_E4, REST, 
  NOTE_C4, REST, NOTE_C4, REST, NOTE_C4, REST, NOTE_C4, REST, 
  NOTE_C4, NOTE_C4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_E4, NOTE_F4, NOTE_F4,   
  NOTE_E4, NOTE_D4, NOTE_C4, REST               
};
const int melodyLength = sizeof(melody) / sizeof(melody[0]);

const int duration[] = {
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  8, 8, 8, 8, 8, 8, 8, 8,
  4, 4, 4, 4
};

int bpm = 90; // 初期値は「普通(90)」
int currentIndex = 0; 
unsigned long nextNoteTime = 0; 
unsigned long stopNoteTime = 0; 
unsigned long noteStartMillis = 0; // 現在の音符が鳴り始めた時刻を記録
float currentNoteDurationMs = 0;   // 現在の音符の総長さ(ms)
bool isPlaying = false;

extern void initTrumpet();
extern void initOrgan();
extern void trumpetPlay(float freq);
extern void trumpetStop();
extern void trumpetUpdate();
extern void organPlay(float freq);
extern void organStop();
extern void organUpdate();
extern float getTrumpetAmp();
extern float getOrganAmp();

float lerp(float start, float end, float amt) {
  return start + amt * (end - start);
}

void setup() {
  Serial.begin(115200);
  initTrumpet();
  initOrgan();
}

void loop() {
  // シリアル通信経由での各種コマンド受付（演奏中も常に監視）
  if (Serial.available() > 0) {
    char c = Serial.read();
    
    if (c == 'S' && !isPlaying) {
      currentIndex = 0;
      isPlaying = true;
      nextNoteTime = millis();
      stopNoteTime = millis();
    } 
    // 🎹 演奏途中でもリアルタイムにBPMを切り替えるテスト処理
    else if (c == '1') {
      bpm = 70;  // 遅い
      recalculateTempo();
    } else if (c == '2') {
      bpm = 90;  // 普通
      recalculateTempo();
    } else if (c == '3') {
      bpm = 110; // 早い
      recalculateTempo();
    }
  }

  if (isPlaying) {
    trumpetUpdate();
    organUpdate();

    // 先行消音（85%ルール）の判定
    if (millis() > stopNoteTime) {
      trumpetStop();
      organStop();
    }
    
    // 次の音符の発音判定
    if (millis() > nextNoteTime) {
      playNextNote();
    }

    // Processing側へ、現在の「周波数」「トランペット音量」「オルガン音量」「現在のBPM」を送信
    Serial.print(currentIndex > 0 ? melody[currentIndex - 1] : 0.0);
    Serial.print(",");
    Serial.print(getTrumpetAmp(), 4);
    Serial.print(",");
    Serial.print(getOrganAmp(), 4);
    Serial.print(",");
    Serial.println(bpm); // 現在のBPM状態もフィードバック
  } else {
    Serial.print("0.0,0.0,0.0,");
    Serial.println(bpm);
  }
  
  delay(10);
}

// 演奏途中でBPMが変わった際，現在鳴っている音符の残りの長さを破綻なく再計算する関数
void recalculateTempo() {
  if (!isPlaying || currentIndex == 0) return;
  
  // 現在鳴らしている音符（currentIndex - 1）の新しいBPM基準での総長さを計算
  int idx = currentIndex - 1;
  float newDurationMs = (60000.0 / bpm) * (4.0 / duration[idx]);
  
  // この音符が始まってからすでに経過した時間を割り出す
  unsigned long elapsed = millis() - noteStartMillis;
  
  // 新しいテンポ基準でタイムラインを上書き
  currentNoteDurationMs = newDurationMs;
  nextNoteTime = noteStartMillis + (unsigned long)currentNoteDurationMs;
  stopNoteTime = noteStartMillis + (unsigned long)(currentNoteDurationMs * 0.85f);
}

void playNextNote() {
  if (currentIndex >= melodyLength) {
    trumpetStop();
    organStop();
    isPlaying = false; 
    return;            
  }
  
  float currentFreq = melody[currentIndex];
  noteStartMillis = millis(); // 発音開始時刻を記録
  
  // 音符の長さをミリ秒に変換
  currentNoteDurationMs = (60000.0 / bpm) * (4.0 / duration[currentIndex]);
  
  nextNoteTime = noteStartMillis + (unsigned long)currentNoteDurationMs;
  stopNoteTime = noteStartMillis + (unsigned long)(currentNoteDurationMs * 0.85f);
  
  if (currentFreq > 0) {
    // 🎹 現在はオルガンでテスト中（必要に応じて入れ替える）
    trumpetStop();
    organPlay(currentFreq);
  } else {
    trumpetStop();
    organStop();
  }
  
  currentIndex++; 
}
