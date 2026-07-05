#include <SPI.h>
#include <AD9833.h>
#include <math.h>

//==================================================
// AD9833
//==================================================
#define DDS1_CS 5
#define DDS2_CS 17

AD9833 dds1(DDS1_CS);
AD9833 dds2(DDS2_CS);

//==================================================
// Frequency
//==================================================
const float FREQ1 = 5000.0;      // Measurement DDS
const float FREQ2 = 5100.0;      // Reference DDS
const float BEAT  = 100.0;       // Beat frequency

//==================================================
// ADC
//==================================================
const int ADC1_PIN = 34;
const int ADC2_PIN = 35;

//==================================================
// Sampling
//==================================================
const int SAMPLE_NUM = 40;
const float SAMPLE_INTERVAL_US = 250.0;
const float SAMPLE_RATE = 4000.0;

//==================================================
// Sample Buffers
//==================================================
float sample1[SAMPLE_NUM];
float sample2[SAMPLE_NUM];

//==================================================
// Reference LUT
//==================================================
float refCos[SAMPLE_NUM];
float refSin[SAMPLE_NUM];

//==================================================
// Results
//==================================================
float I1, Q1, amplitude1, phase1;
float I2, Q2, amplitude2, phase2;

//==================================================
// IQ Detection Function
//==================================================
void IQdetect(float sample[],
              float &I,
              float &Q,
              float &amplitude,
              float &phaseDeg)
{
    //------------------------
    // Remove DC Offset
    //------------------------
    float offset = 0;

    for (int i = 0; i < SAMPLE_NUM; i++)
        offset += sample[i];

    offset /= SAMPLE_NUM;

    for (int i = 0; i < SAMPLE_NUM; i++)
        sample[i] -= offset;

    //------------------------
    // IQ Detection
    //------------------------
    I = 0;
    Q = 0;

    for (int i = 0; i < SAMPLE_NUM; i++)
    {
        I += sample[i] * refCos[i];
        Q += sample[i] * refSin[i];
    }

    I /= SAMPLE_NUM;
    Q /= SAMPLE_NUM;

    //------------------------
    // Amplitude
    //------------------------
    amplitude = 2.0 * sqrt(I * I + Q * Q);

    //------------------------
    // Phase
    //------------------------
    phaseDeg = atan2(Q, I) * 180.0 / PI;
}

//==================================================
// Setup
//==================================================
void setup()
{
    Serial.begin(115200);

    //------------------------
    // SPI
    //------------------------
    SPI.begin(18, -1, 23, -1);

    //------------------------
    // DDS
    //------------------------
    dds1.begin();
    dds2.begin();

    dds1.setWave(AD9833_SINE);
    dds2.setWave(AD9833_SINE);

    dds1.setFrequency(FREQ1);
    dds2.setFrequency(FREQ2);

    dds1.setPhase(0);
    dds2.setPhase(0);

    //------------------------
    // ADC
    //------------------------
    analogReadResolution(12);

    //------------------------
    // Reference Table
    //------------------------
    for (int i = 0; i < SAMPLE_NUM; i++)
    {
        float t = (float)i / SAMPLE_RATE;

        refCos[i] = cosf(2.0f * PI * BEAT * t);
        refSin[i] = sinf(2.0f * PI * BEAT * t);
    }

    delay(1000);

    Serial.println("Start");
}

//==================================================
// Loop
//==================================================
void loop()
{
    //------------------------
    // Sampling
    //------------------------
    uint32_t start = micros();

    for (int i = 0; i < SAMPLE_NUM; i++)
    {
        while (micros() - start < (uint32_t)(i * SAMPLE_INTERVAL_US));

        sample1[i] = analogRead(ADC1_PIN);
        sample2[i] = analogRead(ADC2_PIN);
    }

    //------------------------
    // IQ Detection
    //------------------------
    IQdetect(sample1, I1, Q1, amplitude1, phase1);
    IQdetect(sample2, I2, Q2, amplitude2, phase2);

    //------------------------
    // Serial Plotter
    //------------------------
    Serial.print(amplitude1);
    Serial.print('\t');

    Serial.print(phase1);
    Serial.print('\t');

    Serial.print(amplitude2);
    Serial.print('\t');

    Serial.print(phase2);
    Serial.print('\t');

    // 縦軸固定用
    Serial.print(200);
    Serial.print('\t');
    Serial.println(-200);

    delay(10);
}
