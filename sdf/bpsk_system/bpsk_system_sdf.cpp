# include "netxpto_20180418.h"

# include "m_qam_transmitter_20180118.h"
# include "i_homodyne_receiver_20180124.h"
# include "bit_decider_20170818.h"
# include "bit_error_rate_20171810.h"
# include "local_oscillator_20180130.h"
# include "balanced_beam_splitter_20180124.h"
# include "photodiode_old_20180118.h"
# include "ti_amplifier_20180102.h"
# include "sampler_20171116.h"
# include "sink_20180118.h"


// #####################################################################################################
// #################################### System Input Parameters ########################################
// #####################################################################################################


class BPSKParameters : public SystemParameters {
public:

	//PARAMETERS
	int numberOfBitsReceived{ -1 };
	int numberOfBitsGenerated{ 1000 };
	int samplesPerSymbol = 16;
	int pLength = 5;
	double bitPeriod = 20e-12;
	double rollOffFactor = 0.3;
	double signalOutputPower_dBm = -20;
	double localOscillatorPower_dBm = 0;
	double localOscillatorPhase = 0;
	vector<t_iqValues> iqAmplitudeValues = { { -1, 0 } ,{ 1, 0 } };
	array<t_complex, 4> transferMatrix = { { 1 / sqrt(2), 1 / sqrt(2), 1 / sqrt(2), -1 / sqrt(2) } };
	double responsivity = 1;
	double amplification = 1e6;
	double electricalNoiseAmplitude = 5e-4*sqrt(2);
	int samplesToSkip = 8 * samplesPerSymbol;
	int bufferLength = 20;
	bool shotNoise = false;

	//METHODS
	/* Returns 'param' filled with the values found in the file 'filename' */
	void readSystemInputParameters(string filename);
	/* Empty Constructor in case you want to read BPSK parameters from a file*/
	BPSKParameters() {}
};

int main(){	
	
	int numberOfBitsReceived = -1;
	int numberOfBitsGenerated = 1000;
	int samplesPerSymbol = 16;
	int pLength = 5;
	double bitPeriod = 20e-12;
	double rollOffFactor = 0.3;
	double signalOutputPower_dBm = -20;
	double localOscillatorPower_dBm = 0;
	double localOscillatorPhase = 0;
	vector<t_iqValues> iqAmplitudeValues = { { -1, 0 } ,{ 1, 0 } };
	array<t_complex, 4> transferMatrix = { { 1 / sqrt(2), 1 / sqrt(2), 1 / sqrt(2), -1 / sqrt(2) } };
	double responsivity = 1;
	double amplification = 1e6;
	double electricalNoiseAmplitude = 5e-4*sqrt(2);
	int samplesToSkip = 8 * samplesPerSymbol;
	int bufferLength = 20;
	bool shotNoise = false;

	BPSKParameters param = BPSKParameters();
	param.readSystemInputParameters("data.txt");
	cout << param.samplesPerSymbol << endl;
	cout << param.numberOfBitsReceived << endl;

	// #####################################################################################################
	// ########################### Signals Declaration and Inicialization ##################################
	// #####################################################################################################

	Binary S0("S0.sgn");
	S0.setBufferLength(param.bufferLength);

	OpticalSignal S1("S1.sgn");
	S1.setBufferLength(param.bufferLength);

	OpticalSignal S2("S2.sgn");
	S2.setBufferLength(param.bufferLength);

	OpticalSignal S3("S3.sgn");
	S3.setBufferLength(param.bufferLength);

	OpticalSignal S4("S4.sgn");
	S4.setBufferLength(param.bufferLength);

	TimeContinuousAmplitudeContinuousReal S5("S5.sgn");
	S5.setBufferLength(param.bufferLength);
	
	TimeDiscreteAmplitudeContinuousReal S6("S6.sgn");
	S6.setBufferLength(param.bufferLength);

	Binary S7("S7.sgn");
	S7.setBufferLength(param.bufferLength);

	Binary S8("S8.sgn");
	S8.setBufferLength(param.bufferLength);

	// #####################################################################################################
	// ########################### Blocks Declaration and Inicialization ###################################
	// #####################################################################################################

	MQamTransmitter B1{ vector<Signal*> {}, vector<Signal*> {&S1, &S0} };
	B1.setNumberOfBits(param.numberOfBitsGenerated);
	B1.setOutputOpticalPower_dBm(param.signalOutputPower_dBm);
	B1.setMode(PseudoRandom);
	B1.setBitPeriod(param.bitPeriod);
	B1.setPatternLength(param.pLength);
	B1.setIqAmplitudes(param.iqAmplitudeValues);
	B1.setNumberOfSamplesPerSymbol(param.samplesPerSymbol);
	B1.setRollOffFactor(param.rollOffFactor);
	B1.setSaveInternalSignals(true);
	B1.setPulseShaperFilter(RaisedCosine);
	B1.setSeeBeginningOfImpulseResponse(false);

	LocalOscillator B2{ vector<Signal*> { }, vector<Signal*> { &S2 } };
	B2.setOpticalPower_dBm(param.localOscillatorPower_dBm);
	B2.setPhase(param.localOscillatorPhase);
	B2.setSamplingPeriod(param.bitPeriod / param.samplesPerSymbol);
	B2.setSymbolPeriod(param.bitPeriod);

	BalancedBeamSplitter B3{ vector<Signal*> {&S1, &S2}, vector<Signal*> {&S3, &S4} };
	B3.setTransferMatrix(param.transferMatrix);

	I_HomodyneReceiver B4{ vector<Signal*> {&S3, &S4}, vector<Signal*> {&S5} };
	B4.useShotNoise(true);
	B4.setElectricalNoiseSpectralDensity(param.electricalNoiseAmplitude);
	B4.setGain(param.amplification);
	B4.setResponsivity(param.responsivity);
	B4.setSaveInternalSignals(true);

	Sampler B5{ vector<Signal*> {&S5}, vector<Signal*> {&S6} };
	B5.setSamplesToSkip(param.samplesToSkip);
	
	BitDecider B6{ vector<Signal*> {&S6}, vector<Signal*> {&S7} };

	BitErrorRate B7{ vector<Signal*> {&S0, &S7}, vector<Signal*> {&S8} };

	Sink B8{ vector<Signal*> { &S8 }, vector<Signal*> {} };
	B8.setNumberOfSamples(param.numberOfBitsGenerated);
	B8.setDisplayNumberOfSamples(true);


	// #####################################################################################################
	// ########################### System Declaration and Inicialization ###################################
	// #####################################################################################################

	System MainSystem{ vector<Block*> { &B1, &B2, &B3, &B4, &B5, &B6, &B7, &B8} };

	// #####################################################################################################
	// #################################### System Run #####################################################
	// #####################################################################################################

	MainSystem.run();

	return 0;

}

void BPSKParameters::readSystemInputParameters(string filename)
{
	ifstream inputFile("./" + filename);
	if (!inputFile) {
		cerr << "ERROR: Could not open " << filename;
		exit(1);
		
	}
	int errorLine = 1;
	//Reads each line
	string line;
	while (getline(inputFile, line)) {
		try {
				//If the line if a comment, it just skips to the next one
				if (string(line).substr(0, 2) != "//") { //Lines that start by // are comments
				vector<string> splitline = split(line, ':');
				if (splitline.at(0) == "numberOfBitsReceived") {
					numberOfBitsReceived = parseInt(splitline.at(1));
				}
				else if (splitline.at(0) == "numberOfBitsGenerated") {
					numberOfBitsGenerated = parseInt(splitline.at(1));
				}
				else if (splitline.at(0) == "samplesPerSymbol") {
					samplesPerSymbol = parseInt(splitline.at(1));
				}
				else if (splitline.at(0) == "pLength") {
					pLength = parseInt(splitline.at(1));
				}
				else if (splitline.at(0) == "bitPeriod") {
					bitPeriod = parseDouble(splitline.at(1));
				}
				else if (splitline.at(0) == "rollOffFactor") {
					rollOffFactor = parseDouble(splitline.at(1));
				}
				else if (splitline.at(0) == "signalOutputPower_dBm") {
					signalOutputPower_dBm = parseDouble(splitline.at(1));
				}
				else if (splitline.at(0) == "localOscillatorPower_dBm") {
					localOscillatorPower_dBm = parseDouble(splitline.at(1));
				}
				else if (splitline.at(0) == "localOscillatorPhase") {
					localOscillatorPhase = parseDouble(splitline.at(1));
				}
				else if (splitline.at(0) == "responsivity") {
					responsivity = parseDouble(splitline.at(1));
				}
				else if (splitline.at(0) == "amplification") {
					amplification = parseDouble(splitline.at(1));
				}
				else if (splitline.at(0) == "electricalNoiseAmplitude") {
					electricalNoiseAmplitude = parseDouble(splitline.at(1));
				}
				else if (splitline.at(0) == "samplesToSkip") {
					samplesToSkip = parseInt(splitline.at(1));
				}
				else if (splitline.at(0) == "bufferLength") {
					bufferLength = parseInt(splitline.at(1));
				}
				else if (splitline.at(0) == "shotNoise") {
					shotNoise = parseBool(splitline.at(1));
				}
			}
			errorLine++;
		}
		catch (const exception& e) {
			cerr << "ERROR: Invalid input in line " << errorLine << " of " << filename;
			exit(1);
		}
	}
	inputFile.close();
}
