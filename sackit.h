#define AMICLK (916315123)

// note, these aren't the IT limits (smps is 99 afaik)
// but they should be fine for now --GM
#define MAX_ORDERS 256
#define MAX_INSTRUMENTS 256
#define MAX_SAMPLES 256
#define MAX_PATTERNS 256

#define SACKIT_MAX_ACHANNEL 256

#define SACKIT_TEST_AU_BUFFER 1024

typedef struct it_pattern
{
	uint16_t length;
	uint16_t rows;
	uint32_t reserved;
	uint8_t data[65536];
} __attribute__((__packed__)) it_pattern_t;

#define IT_SMP_EXISTS   0x01
#define IT_SMP_16BIT    0x02
#define IT_SMP_STEREO   0x04
#define IT_SMP_COMPRESS 0x08
#define IT_SMP_LOOP     0x10
#define IT_SMP_SUSLOOP  0x20
#define IT_SMP_LOOPBIDI 0x40
#define IT_SMP_SUSBIDI  0x80

typedef struct it_sample
{
	uint8_t magic[4]; // IMPS
	uint8_t dos_filename[13];
	uint8_t gvl, flg, vol;
	uint8_t sample_name[26];
	uint8_t cvt, dfp;
	uint32_t length;
	uint32_t loop_begin, loop_end;
	uint32_t c5speed;
	uint32_t susloop_begin, susloop_end;
	uint32_t samplepointer;
	uint8_t vis,vid,vir,vit;
	int16_t *data;
} __attribute__((__packed__)) it_sample_t;

#define IT_ENV_ON       0x01
#define IT_ENV_LOOP     0x02
#define IT_ENV_SUSLOOP  0x04
#define IT_ENV_CARRY    0x08
#define IT_ENV_FILTER   0x80
// TODO: confirm which bit ENV_CARRY *really* uses
// (it's not in ITTECH - dammit Jeff...)

typedef struct it_envelope
{
	uint8_t flg,num;
	uint8_t lpb,lpe;
	uint8_t slb,sle;
	struct {
		int8_t y;
		uint16_t x;
	} __attribute__((__packed__)) points;
	
} __attribute__((__packed__)) it_envelope_t;

#define IT_MOD_STEREO  0x01
#define IT_MOD_VOL0MIX 0x02 /* Most. Useless. Flag. Ever. */
#define IT_MOD_INSTR   0x04
#define IT_MOD_LINEAR  0x08
#define IT_MOD_OLDFX   0x10
#define IT_MOD_COMPGXX 0x20
#define IT_MOD_USEPWD  0x40
#define IT_MOD_GETMIDI 0x80
#define IT_SPECIAL_MESSAGE  0x01
//define IT_SPECIAL_        0x02 // unknown
//define IT_SPECIAL_        0x04 // unknown
#define IT_SPECIAL_HASMIDI  0x08

typedef struct it_instrument
{
	uint8_t magic[4]; // IMPI
	uint8_t dos_filename[13];
	uint8_t nna,dct,dca;
	uint16_t fadeout;
	uint8_t pps,ppc;
	uint8_t gbv,dfp;
	uint8_t rv,rp;
	uint16_t trkvers;
	uint8_t nos,resv1;
	uint8_t instrument_name[26];
	uint8_t ifc,ifr;
	uint8_t mch,mpr;
	uint16_t midibnk;
	uint8_t notesample[120][2];
	it_envelope_t env_vol;
	it_envelope_t env_pan;
	it_envelope_t env_pitch;
} __attribute__((__packed__)) it_instrument_t;
typedef struct it_module_header it_module_header_t;
typedef struct it_module
{
	struct it_module_header {
		uint8_t magic[4]; // IMPM
		uint8_t song_name[26];
		uint16_t philigt;
		uint16_t ordnum, insnum, smpnum, patnum;
		uint16_t cwtv, cmwt;
		uint16_t flags, special;
		uint8_t gv, mv, is, it, sep, pwd;
		uint16_t msglgth;
		uint32_t message_offset;
		uint32_t timestamp; // reserved my ass --GM
		uint8_t chnl_pan[64];
		uint8_t chnl_vol[64];
	} __attribute__((__packed__)) header;
	uint8_t orders[MAX_ORDERS];
	it_instrument_t *instruments[MAX_INSTRUMENTS];
	it_sample_t *samples[MAX_SAMPLES];
	it_pattern_t *patterns[MAX_PATTERNS];
} it_module_t;

// audio channel
#define SACKIT_ACHN_PLAYING  0x01
#define SACKIT_ACHN_MIXING   0x02
#define SACKIT_ACHN_RAMP     0x04
#define SACKIT_ACHN_REVERSE  0x08

typedef struct sackit_achannel
{
	int32_t ofreq;
	int32_t freq;
	int32_t offs;
	int32_t suboffs;
	uint16_t flags;
	uint8_t vol,sv,cv; // TODO: more stuff
	uint8_t fv;
	it_instrument_t *instrument;
	it_sample_t *sample;
} sackit_achannel_t;

// pattern channel
typedef struct sackit_pchannel
{
	sackit_achannel_t *achn;
	
	uint32_t tfreq;
	uint32_t nfreq;
	uint8_t note;
	uint8_t lins;
	uint8_t cv;
	
	int16_t slide_vol;
	int16_t slide_pan;
	int16_t slide_pitch;
	int16_t slide_porta;
	int16_t slide_vol_cv;
	int16_t slide_vol_gv;
	uint16_t arpeggio;
	uint16_t note_cut;
	uint16_t note_delay;
	uint16_t note_delay_note;
	uint16_t note_delay_ins;
	uint16_t note_delay_vol;
	uint16_t vib_speed;
	uint16_t vib_depth;
	uint16_t vib_offs;
	uint16_t vib_type;
	uint16_t vib_lins;
	
	uint8_t eff_slide_vol;
	uint8_t eff_slide_vol_cv;
	uint8_t eff_slide_vol_gv;
	uint8_t eff_slide_pitch;
	uint8_t eff_slide_porta;
	uint8_t eff_sample_offs;
	uint8_t eff_misc;
	uint8_t eff_arpeggio;
	uint8_t eff_vibrato;
	uint8_t eff_tempo;
	
	uint8_t lmask,ldata[5];
} sackit_pchannel_t;

typedef struct sackit_playback
{
	it_module_t *module;
	
	uint16_t current_tick;
	uint16_t max_tick;
	uint16_t row_counter;
	
	uint16_t current_row;
	uint16_t process_row;
	uint16_t break_row;
	uint16_t number_of_rows; // TODO? refactor into pattern?
	
	uint16_t current_pattern;
	uint16_t process_order;
	
	uint16_t pat_ptr; // index of next row
	uint16_t pat_row;
	
	uint16_t tempo;
	
	uint32_t buf_len;
	uint32_t buf_tick_rem;
	int16_t *buf;
	int32_t *mixbuf;
	
	uint8_t gv,mv;
	
	uint16_t achn_count;
	sackit_pchannel_t pchn[64];
	sackit_achannel_t achn[SACKIT_MAX_ACHANNEL];
} sackit_playback_t;

// objects.c
it_module_t *sackit_module_new(void);
void sackit_module_free(it_module_t *module);
it_module_t *sackit_module_load(char *fname);
void sackit_playback_free(sackit_playback_t *sackit);
void sackit_playback_reset(sackit_playback_t *sackit);
sackit_playback_t *sackit_playback_new(it_module_t *module);

// playroutine.c
void sackit_playback_update(sackit_playback_t *sackit);
