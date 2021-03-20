#include "alsa/asoundlib.h"
#include <vector>
#include <cmath>
static char *device = "default";            /* playback device */
std::vector<int32_t> buffer;

enum WaveType{
    wave_Sin,
    wave_Saw,
};

// channel - 0(left) or 1(right)
void generateSin(int channel, double freq, double phase, double volume, double samplingRate){
    size_t period = 1/freq * samplingRate;
    size_t phase_shift = period*phase/2;
    size_t counter = phase_shift;
    for (size_t i = channel; i < buffer.size(); i+=2){
        if (i == 108){
            int a = 6;
        }
        buffer[i] = (volume*sin(2*M_PI*freq * (counter/samplingRate)));
	    counter++;
    }
}


// channel - 0(left) or 1(right)
void generateSaw(int channel, double freq, double phase, double volume, double samplingRate){
    size_t period = 1/freq * samplingRate;
    size_t phase_shift = period*phase/2;
    size_t counter = phase_shift;
    double delta = 2.0 / period;
    for (size_t i = channel; i < buffer.size(); i+=2){
        buffer[i] = (volume*delta * static_cast<double>(counter % period) - period*delta/2);
	counter++;
    }
}


void printHelp(){
    printf("Usage:\n");
    printf("  <samplingRate> <waveL> <freqL> <phaseL> <volumeL> <waveR> <freqR> <phaseR> <volumeR>\n\n");
    printf("Example:\n");
    printf("  44100 sin 440 0 50 saw 440 0 50\n\n");
}


WaveType parseWaveType(const char* c){
    if (!strcmp(c, "sin")){
	printf("parsed sin\n");
        return wave_Sin;
    }else if (!strcmp(c, "saw")) {
	printf("parsed saw\n");
        return wave_Saw;
    }
    return wave_Sin;
}

int main(int argc, char* argv[])
{
    int err;
    snd_pcm_t *handle;
    snd_pcm_sframes_t frames;

    double samplingRate = 44100;
    WaveType wTypeL = wave_Sin;
    WaveType wTypeR = wave_Saw;
    double freqL = 440.0, freqR = 440.0;
    double phaseL = 0.0, phaseR = 0.0;
    double volumeL = 50.0 * 1E+7, volumeR = 50.0 * 1E+7;

    if (argc != 10){
        printHelp();

    }else{
        samplingRate = atof(argv[1]);

        wTypeL  = parseWaveType(argv[2]);
        freqL   = atof(argv[3]);
        phaseL  = atof(argv[4]);
        volumeL = atof(argv[5]) * 1E+7;

        wTypeR  = parseWaveType(argv[6]);
        freqR   = atof(argv[7]);
        phaseR  = atof(argv[8]);
        volumeR = atof(argv[9]) * 1E+7;
    }
    
    buffer.resize(2*samplingRate);
    
    // generate Left channel signal
    if (wTypeL == wave_Sin){
        generateSin(0, freqL, phaseL, volumeL, samplingRate);

    }else if (wTypeL == wave_Saw) {
        generateSaw(0, freqL, phaseL, volumeL, samplingRate);
    }

    // generate Right channel signal
    if (wTypeR == wave_Sin){
        generateSin(1, freqR, phaseR, volumeR, samplingRate);

    }else if (wTypeR == wave_Saw) {
        generateSaw(1, freqR, phaseR, volumeR, samplingRate);
    }

    
    if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_set_params(handle,
                      SND_PCM_FORMAT_S32_LE,
                      SND_PCM_ACCESS_RW_INTERLEAVED,
                      2,
                      samplingRate,
                      1,
                      1000000)) < 0) {   /* 1.0 sec */
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    for (; ;) {
        frames = snd_pcm_writei(handle, (void*)(&buffer[0]), buffer.size()/2);
	    printf("frames: %li\n", frames);
        if (frames < 0)
            frames = snd_pcm_recover(handle, frames, 0);

        if (frames < 0) {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
            break;
        }
        if (frames > 0 && frames < buffer.size()/2)
            printf("Short write (expected %li, wrote %li)\n", buffer.size()/2, frames);
    }

    /* pass the remaining samples, otherwise they're dropped in close */
    err = snd_pcm_drain(handle);
    if (err < 0)
        printf("snd_pcm_drain failed: %s\n", snd_strerror(err));

    snd_pcm_close(handle);
    return 0;
}
