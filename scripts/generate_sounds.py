import wave
import struct
import math

def generate_wav(filename, duration, frequency, volume=0.5, sample_rate=44100):
    num_samples = int(duration * sample_rate)
    with wave.open(filename, 'w') as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(sample_rate)
        for i in range(num_samples):
            value = int(volume * 32767.0 * math.sin(2.0 * math.pi * frequency * i / sample_rate))
            wav.writeframesraw(struct.pack('<h', value))

# 0.1s short 'click' for slide
generate_wav("slide.wav", 0.05, 440)
# 0.5s 'ding' for completion
generate_wav("complete.wav", 0.3, 880)
