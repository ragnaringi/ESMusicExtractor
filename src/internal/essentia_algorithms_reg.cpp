#include "algorithmfactory.h"
#include "algorithms/rhythm/rhythmextractor2013.h"
#include "algorithms/rhythm/tempotapticks.h"
#include "algorithms/extractor/rhythmdescriptors.h"
#include "algorithms/spectral/hpcp.h"
#include "algorithms/standard/binaryoperator.h"
#include "algorithms/sfx/maxtototal.h"
#include "algorithms/spectral/maxmagfreq.h"
#include "algorithms/standard/maxfilter.h"
#include "algorithms/spectral/strongpeak.h"
#include "algorithms/spectral/spectrumtocent.h"
#include "algorithms/temporal/loudness.h"
#include "algorithms/standard/stereodemuxer.h"
#include "algorithms/stats/instantpower.h"
#include "algorithms/spectral/erbbands.h"
#include "algorithms/extractor/tuningfrequencyextractor.h"
#include "algorithms/standard/autocorrelation.h"
#include "algorithms/stats/energy.h"
#include "algorithms/standard/dct.h"
#include "algorithms/rhythm/tempotap.h"
#include "algorithms/stats/geometricmean.h"
#include "algorithms/spectral/melbands.h"
#include "algorithms/complex/polartocartesian.h"
#include "algorithms/rhythm/beatsloudness.h"
#include "algorithms/spectral/spectralpeaks.h"
#include "algorithms/standard/trimmer.h"
#include "algorithms/spectral/barkbands.h"
#include "algorithms/standard/monomixer.h"
#include "algorithms/tonal/chordsdescriptors.h"
#include "algorithms/rhythm/onsetdetection.h"
#include "algorithms/stats/variance.h"
#include "algorithms/temporal/lpc.h"
#include "algorithms/sfx/logattacktime.h"
#include "algorithms/rhythm/superfluxextractor.h"
#include "algorithms/standard/slicer.h"
#include "algorithms/spectral/spectralcontrast.h"
#include "algorithms/highlevel/sbic.h"
#include "algorithms/highlevel/intensity.h"
#include "algorithms/rhythm/onsetrate.h"
#include "algorithms/standard/stereomuxer.h"
#include "algorithms/stats/centroid.h"
#include "algorithms/spectral/triangularbands.h"
#include "algorithms/standard/stereotrimmer.h"
#include "algorithms/standard/scale.h"
#include "algorithms/standard/vectorrealaccumulator.h"
#include "algorithms/stats/poolaggregator.h"
#include "algorithms/extractor/lowlevelspectralextractor.h"
#include "algorithms/standard/idct.h"
#include "algorithms/filters/highpass.h"
#include "algorithms/standard/overlapadd.h"
#include "algorithms/temporal/effectiveduration.h"
#include "algorithms/spectral/panning.h"
#include "algorithms/extractor/lowlevelspectraleqloudextractor.h"
#include "algorithms/highlevel/pca.h"
#include "algorithms/standard/startstopsilence.h"
#include "algorithms/sfx/strongdecay.h"
#include "algorithms/rhythm/singlebeatloudness.h"
#include "algorithms/stats/crest.h"
#include "algorithms/io/fileoutputproxy.h"
#include "algorithms/tonal/key.h"
#include "algorithms/filters/equalloudness.h"
#include "algorithms/tonal/pitchfilter.h"
#include "algorithms/standard/warpedautocorrelation.h"
#include "algorithms/standard/replaygain.h"
#include "algorithms/standard/realaccumulator.h"
#include "algorithms/standard/nsgconstantqstreaming.h"
#include "algorithms/rhythm/beattrackerdegara.h"
#include "algorithms/spectral/gfcc.h"
#include "algorithms/rhythm/onsetdetectionglobal.h"
#include "algorithms/spectral/spectralcentroidtime.h"
#include "algorithms/spectral/energybandratio.h"
#include "algorithms/standard/iffta.h"
#include "algorithms/spectral/flatnessdb.h"
#include "algorithms/io/audioonsetsmarker.h"
#include "algorithms/extractor/keyextractor.h"
#include "algorithms/stats/flatness.h"
#include "algorithms/rhythm/tempotapdegara.h"
#include "algorithms/rhythm/bpmhistogram.h"
#include "algorithms/standard/clipper.h"
#include "algorithms/standard/derivative.h"
#include "algorithms/standard/noiseadder.h"
#include "algorithms/rhythm/percivalbpmestimator.h"
#include "algorithms/spectral/spectralcomplexity.h"
#include "algorithms/rhythm/harmonicbpm.h"
#include "algorithms/highlevel/danceability.h"
#include "algorithms/tonal/tuningfrequency.h"
#include "algorithms/spectral/hfc.h"
#include "algorithms/stats/rms.h"
#include "algorithms/temporal/loudnessebur128.h"
#include "algorithms/standard/bpf.h"
#include "algorithms/standard/spectrum.h"
#include "algorithms/standard/windowing.h"
#include "algorithms/rhythm/rhythmextractor.h"
#include "algorithms/stats/mean.h"
#include "algorithms/rhythm/onsets.h"
#include "algorithms/standard/framecutter.h"
#include "algorithms/spectral/frequencybands.h"
#include "algorithms/spectral/energyband.h"
#include "algorithms/rhythm/bpmhistogramdescriptors.h"
#include "algorithms/filters/movingaverage.h"
#include "algorithms/complex/magnitude.h"
#include "algorithms/stats/distributionshape.h"
#include "algorithms/sfx/mintototal.h"
#include "algorithms/stats/centralmoments.h"
#include "algorithms/standard/spline.h"
#include "algorithms/extractor/levelextractor.h"
#include "algorithms/stats/decrease.h"
#include "algorithms/standard/peakdetection.h"
#include "algorithms/standard/ffta.h"
#include "algorithms/tonal/dissonance.h"
#include "algorithms/standard/multiplexer.h"
#include "algorithms/standard/binaryoperatorstream.h"
#include "algorithms/tonal/highresolutionfeatures.h"
#include "algorithms/rhythm/loopbpmconfidence.h"
#include "algorithms/complex/cartesiantopolar.h"
#include "algorithms/temporal/zerocrossingrate.h"
#include "algorithms/rhythm/tempotapmaxagreement.h"
#include "algorithms/rhythm/beattrackermultifeature.h"
#include "algorithms/spectral/rolloff.h"
#include "algorithms/rhythm/bpmrubato.h"
#include "algorithms/rhythm/loopbpmestimator.h"
#include "algorithms/extractor/tonalextractor.h"
#include "algorithms/filters/iir.h"
#include "algorithms/rhythm/superfluxpeaks.h"
#include "algorithms/temporal/loudnessebur128filter.h"
#include "algorithms/standard/envelope.h"
#include "algorithms/standard/silencerate.h"
#include "algorithms/sfx/tctototal.h"
#include "algorithms/sfx/pitchsalience.h"
#include "algorithms/standard/unaryoperator.h"
#include "algorithms/stats/entropy.h"
#include "algorithms/rhythm/rhythmtransform.h"
#include "algorithms/spectral/bfcc.h"
#include "algorithms/spectral/flux.h"
#include "algorithms/standard/crosscorrelation.h"
#include "algorithms/highlevel/fadedetection.h"
#include "algorithms/rhythm/temposcalebands.h"
#include "algorithms/stats/singlegaussian.h"
#include "algorithms/highlevel/dynamiccomplexity.h"
#include "algorithms/extractor/extractor.h"
#include "algorithms/standard/unaryoperatorstream.h"
#include "algorithms/rhythm/noveltycurvefixedbpmestimator.h"
#include "algorithms/tonal/chordsdetection.h"
#include "algorithms/tonal/pitchyin.h"
#include "algorithms/spectral/mfcc.h"
#include "algorithms/standard/frametoreal.h"
#include "algorithms/standard/powerspectrum.h"
#include "algorithms/temporal/duration.h"
#include "algorithms/standard/cubicspline.h"
#include "algorithms/extractor/musicextractor.h"
#include "algorithms/io/easyloader.h"
#include "algorithms/io/eqloudloader.h"
#include "algorithms/io/monoloader.h"
#include "algorithms/io/metadatareader.h"
#include "algorithms/io/yamloutput.h"
#include "algorithms/standard/resample.h"
#include "audioloader_ios.h"

namespace essentia {
namespace standard {

ESSENTIA_API void registerAlgorithm() {
    AlgorithmFactory::Registrar<RhythmExtractor2013> regRhythmExtractor2013;
    AlgorithmFactory::Registrar<TempoTapTicks> regTempoTapTicks;
    AlgorithmFactory::Registrar<RhythmDescriptors> regRhythmDescriptors;
    AlgorithmFactory::Registrar<HPCP> regHPCP;
    AlgorithmFactory::Registrar<BinaryOperator> regBinaryOperator;
    AlgorithmFactory::Registrar<MaxToTotal> regMaxToTotal;
    AlgorithmFactory::Registrar<MaxMagFreq> regMaxMagFreq;
    AlgorithmFactory::Registrar<MaxFilter> regMaxFilter;
    AlgorithmFactory::Registrar<StrongPeak> regStrongPeak;
    AlgorithmFactory::Registrar<SpectrumToCent> regSpectrumToCent;
    AlgorithmFactory::Registrar<Loudness> regLoudness;
    AlgorithmFactory::Registrar<StereoDemuxer> regStereoDemuxer;
    AlgorithmFactory::Registrar<InstantPower> regInstantPower;
    AlgorithmFactory::Registrar<ERBBands> regERBBands;
    AlgorithmFactory::Registrar<TuningFrequencyExtractor> regTuningFrequencyExtractor;
    AlgorithmFactory::Registrar<AutoCorrelation> regAutoCorrelation;
    AlgorithmFactory::Registrar<Energy> regEnergy;
    AlgorithmFactory::Registrar<DCT> regDCT;
    AlgorithmFactory::Registrar<TempoTap> regTempoTap;
    AlgorithmFactory::Registrar<GeometricMean> regGeometricMean;
    AlgorithmFactory::Registrar<MelBands> regMelBands;
    AlgorithmFactory::Registrar<PolarToCartesian> regPolarToCartesian;
    AlgorithmFactory::Registrar<BeatsLoudness> regBeatsLoudness;
    AlgorithmFactory::Registrar<SpectralPeaks> regSpectralPeaks;
    AlgorithmFactory::Registrar<Trimmer> regTrimmer;
    AlgorithmFactory::Registrar<BarkBands> regBarkBands;
    AlgorithmFactory::Registrar<MonoMixer> regMonoMixer;
    AlgorithmFactory::Registrar<ChordsDescriptors> regChordsDescriptors;
    AlgorithmFactory::Registrar<OnsetDetection> regOnsetDetection;
    AlgorithmFactory::Registrar<LPC> regLPC;
    AlgorithmFactory::Registrar<LogAttackTime> regLogAttackTime;
    AlgorithmFactory::Registrar<SuperFluxExtractor> regSuperFluxExtractor;
    AlgorithmFactory::Registrar<Slicer> regSlicer;
    AlgorithmFactory::Registrar<SpectralContrast> regSpectralContrast;
    AlgorithmFactory::Registrar<SBic> regSBic;
    AlgorithmFactory::Registrar<Intensity> regIntensity;
    AlgorithmFactory::Registrar<OnsetRate> regOnsetRate;
    AlgorithmFactory::Registrar<StereoMuxer> regStereoMuxer;
    AlgorithmFactory::Registrar<Centroid> regCentroid;
    AlgorithmFactory::Registrar<TriangularBands> regTriangularBands;
    AlgorithmFactory::Registrar<StereoTrimmer> regStereoTrimmer;
    AlgorithmFactory::Registrar<Scale> regScale;
    AlgorithmFactory::Registrar<PoolAggregator> regPoolAggregator;
    AlgorithmFactory::Registrar<LowLevelSpectralExtractor> regLowLevelSpectralExtractor;
    AlgorithmFactory::Registrar<IDCT> regIDCT;
    AlgorithmFactory::Registrar<OverlapAdd> regOverlapAdd;
    AlgorithmFactory::Registrar<EffectiveDuration> regEffectiveDuration;
    AlgorithmFactory::Registrar<Panning> regPanning;
    AlgorithmFactory::Registrar<LowLevelSpectralEqloudExtractor> regLowLevelSpectralEqloudExtractor;
    AlgorithmFactory::Registrar<PCA> regPCA;
    AlgorithmFactory::Registrar<StartStopSilence> regStartStopSilence;
    AlgorithmFactory::Registrar<StrongDecay> regStrongDecay;
    AlgorithmFactory::Registrar<SingleBeatLoudness> regSingleBeatLoudness;
    AlgorithmFactory::Registrar<Crest> regCrest;
    AlgorithmFactory::Registrar<Key> regKey;
    AlgorithmFactory::Registrar<EqualLoudness> regEqualLoudness;
    AlgorithmFactory::Registrar<WarpedAutoCorrelation> regWarpedAutoCorrelation;
    AlgorithmFactory::Registrar<ReplayGain> regReplayGain;
    AlgorithmFactory::Registrar<BeatTrackerDegara> regBeatTrackerDegara;
    AlgorithmFactory::Registrar<GFCC> regGFCC;
    AlgorithmFactory::Registrar<OnsetDetectionGlobal> regOnsetDetectionGlobal;
    AlgorithmFactory::Registrar<SpectralCentroidTime> regSpectralCentroidTime;
    AlgorithmFactory::Registrar<EnergyBandRatio> regEnergyBandRatio;
    AlgorithmFactory::Registrar<IFFTA> regIFFTA;
    AlgorithmFactory::Registrar<FlatnessDB> regFlatnessDB;
    AlgorithmFactory::Registrar<AudioOnsetsMarker> regAudioOnsetsMarker;
    AlgorithmFactory::Registrar<KeyExtractor> regKeyExtractor;
    AlgorithmFactory::Registrar<Flatness> regFlatness;
    AlgorithmFactory::Registrar<TempoTapDegara> regTempoTapDegara;
    AlgorithmFactory::Registrar<BpmHistogram> regBpmHistogram;
    AlgorithmFactory::Registrar<Clipper> regClipper;
    AlgorithmFactory::Registrar<Derivative> regDerivative;
    AlgorithmFactory::Registrar<NoiseAdder> regNoiseAdder;
    AlgorithmFactory::Registrar<PercivalBpmEstimator> regPercivalBpmEstimator;
    AlgorithmFactory::Registrar<SpectralComplexity> regSpectralComplexity;
    AlgorithmFactory::Registrar<HarmonicBpm> regHarmonicBpm;
    AlgorithmFactory::Registrar<Danceability> regDanceability;
    AlgorithmFactory::Registrar<TuningFrequency> regTuningFrequency;
    AlgorithmFactory::Registrar<HFC> regHFC;
    AlgorithmFactory::Registrar<RMS> regRMS;
    AlgorithmFactory::Registrar<LoudnessEBUR128> regLoudnessEBUR128;
    AlgorithmFactory::Registrar<BPF> regBPF;
    AlgorithmFactory::Registrar<Spectrum> regSpectrum;
    AlgorithmFactory::Registrar<Windowing> regWindowing;
    AlgorithmFactory::Registrar<RhythmExtractor> regRhythmExtractor;
    AlgorithmFactory::Registrar<Mean> regMean;
    AlgorithmFactory::Registrar<Onsets> regOnsets;
    AlgorithmFactory::Registrar<FrameCutter> regFrameCutter;
    AlgorithmFactory::Registrar<FrequencyBands> regFrequencyBands;
    AlgorithmFactory::Registrar<EnergyBand> regEnergyBand;
    AlgorithmFactory::Registrar<BpmHistogramDescriptors> regBpmHistogramDescriptors;
    AlgorithmFactory::Registrar<MovingAverage> regMovingAverage;
    AlgorithmFactory::Registrar<Magnitude> regMagnitude;
    AlgorithmFactory::Registrar<DistributionShape> regDistributionShape;
    AlgorithmFactory::Registrar<MinToTotal> regMinToTotal;
    AlgorithmFactory::Registrar<CentralMoments> regCentralMoments;
    AlgorithmFactory::Registrar<Spline> regSpline;
    AlgorithmFactory::Registrar<LevelExtractor> regLevelExtractor;
    AlgorithmFactory::Registrar<Decrease> regDecrease;
    AlgorithmFactory::Registrar<PeakDetection> regPeakDetection;
    AlgorithmFactory::Registrar<FFTA> regFFTA;
    AlgorithmFactory::Registrar<Dissonance> regDissonance;
    AlgorithmFactory::Registrar<Multiplexer> regMultiplexer;
    AlgorithmFactory::Registrar<BinaryOperatorStream> regBinaryOperatorStream;
    AlgorithmFactory::Registrar<HighResolutionFeatures> regHighResolutionFeatures;
    AlgorithmFactory::Registrar<LoopBpmConfidence> regLoopBpmConfidence;
    AlgorithmFactory::Registrar<CartesianToPolar> regCartesianToPolar;
    AlgorithmFactory::Registrar<ZeroCrossingRate> regZeroCrossingRate;
    AlgorithmFactory::Registrar<TempoTapMaxAgreement> regTempoTapMaxAgreement;
    AlgorithmFactory::Registrar<BeatTrackerMultiFeature> regBeatTrackerMultiFeature;
    AlgorithmFactory::Registrar<RollOff> regRollOff;
    AlgorithmFactory::Registrar<BpmRubato> regBpmRubato;
    AlgorithmFactory::Registrar<LoopBpmEstimator> regLoopBpmEstimator;
    AlgorithmFactory::Registrar<TonalExtractor> regTonalExtractor;
    AlgorithmFactory::Registrar<IIR> regIIR;
    AlgorithmFactory::Registrar<SuperFluxPeaks> regSuperFluxPeaks;
    AlgorithmFactory::Registrar<Envelope> regEnvelope;
    AlgorithmFactory::Registrar<SilenceRate> regSilenceRate;
    AlgorithmFactory::Registrar<TCToTotal> regTCToTotal;
    AlgorithmFactory::Registrar<PitchSalience> regPitchSalience;
    AlgorithmFactory::Registrar<UnaryOperator> regUnaryOperator;
    AlgorithmFactory::Registrar<Entropy> regEntropy;
    AlgorithmFactory::Registrar<RhythmTransform> regRhythmTransform;
    AlgorithmFactory::Registrar<BFCC> regBFCC;
    AlgorithmFactory::Registrar<Flux> regFlux;
    AlgorithmFactory::Registrar<CrossCorrelation> regCrossCorrelation;
    AlgorithmFactory::Registrar<FadeDetection> regFadeDetection;
    AlgorithmFactory::Registrar<TempoScaleBands> regTempoScaleBands;
    AlgorithmFactory::Registrar<SingleGaussian> regSingleGaussian;
    AlgorithmFactory::Registrar<DynamicComplexity> regDynamicComplexity;
    AlgorithmFactory::Registrar<Extractor> regExtractor;
    AlgorithmFactory::Registrar<UnaryOperatorStream> regUnaryOperatorStream;
    AlgorithmFactory::Registrar<NoveltyCurveFixedBpmEstimator> regNoveltyCurveFixedBpmEstimator;
    AlgorithmFactory::Registrar<ChordsDetection> regChordsDetection;
    AlgorithmFactory::Registrar<MFCC> regMFCC;
    AlgorithmFactory::Registrar<FrameToReal> regFrameToReal;
    AlgorithmFactory::Registrar<PowerSpectrum> regPowerSpectrum;
    AlgorithmFactory::Registrar<Duration> regDuration;
    AlgorithmFactory::Registrar<CubicSpline> regCubicSpline;
    AlgorithmFactory::Registrar<MusicExtractor> regMusicExtractor;
    AlgorithmFactory::Registrar<AudioLoader> regAudioLoader;
    AlgorithmFactory::Registrar<EasyLoader> regEasyLoader;
    AlgorithmFactory::Registrar<EqloudLoader> regEqloudLoader;
    AlgorithmFactory::Registrar<MonoLoader> regMonoLoader;
    AlgorithmFactory::Registrar<MetadataReader> regMetaDataReader;
    AlgorithmFactory::Registrar<Resample> regResample;
    AlgorithmFactory::Registrar<YamlOutput> regYamlOutput;
}}}



namespace essentia {
namespace streaming {

ESSENTIA_API void registerAlgorithm() {
    AlgorithmFactory::Registrar<RhythmExtractor2013, essentia::standard::RhythmExtractor2013> regRhythmExtractor2013;
    AlgorithmFactory::Registrar<TempoTapTicks, essentia::standard::TempoTapTicks> regTempoTapTicks;
    AlgorithmFactory::Registrar<RhythmDescriptors, essentia::standard::RhythmDescriptors> regRhythmDescriptors;
    AlgorithmFactory::Registrar<HPCP, essentia::standard::HPCP> regHPCP;
    AlgorithmFactory::Registrar<BinaryOperator, essentia::standard::BinaryOperator> regBinaryOperator;
    AlgorithmFactory::Registrar<MaxToTotal, essentia::standard::MaxToTotal> regMaxToTotal;
    AlgorithmFactory::Registrar<MaxMagFreq, essentia::standard::MaxMagFreq> regMaxMagFreq;
    AlgorithmFactory::Registrar<MaxFilter, essentia::standard::MaxFilter> regMaxFilter;
    AlgorithmFactory::Registrar<StrongPeak, essentia::standard::StrongPeak> regStrongPeak;
    AlgorithmFactory::Registrar<SpectrumToCent, essentia::standard::SpectrumToCent> regSpectrumToCent;
    AlgorithmFactory::Registrar<Loudness, essentia::standard::Loudness> regLoudness;
    AlgorithmFactory::Registrar<StereoDemuxer, essentia::standard::StereoDemuxer> regStereoDemuxer;
    AlgorithmFactory::Registrar<InstantPower, essentia::standard::InstantPower> regInstantPower;
    AlgorithmFactory::Registrar<ERBBands, essentia::standard::ERBBands> regERBBands;
    AlgorithmFactory::Registrar<TuningFrequencyExtractor, essentia::standard::TuningFrequencyExtractor> regTuningFrequencyExtractor;
    AlgorithmFactory::Registrar<AutoCorrelation, essentia::standard::AutoCorrelation> regAutoCorrelation;
    AlgorithmFactory::Registrar<Energy, essentia::standard::Energy> regEnergy;
    AlgorithmFactory::Registrar<DCT, essentia::standard::DCT> regDCT;
    AlgorithmFactory::Registrar<TempoTap, essentia::standard::TempoTap> regTempoTap;
    AlgorithmFactory::Registrar<GeometricMean, essentia::standard::GeometricMean> regGeometricMean;
    AlgorithmFactory::Registrar<MelBands, essentia::standard::MelBands> regMelBands;
    AlgorithmFactory::Registrar<PolarToCartesian, essentia::standard::PolarToCartesian> regPolarToCartesian;
    AlgorithmFactory::Registrar<BeatsLoudness, essentia::standard::BeatsLoudness> regBeatsLoudness;
    AlgorithmFactory::Registrar<SpectralPeaks, essentia::standard::SpectralPeaks> regSpectralPeaks;
    AlgorithmFactory::Registrar<Trimmer, essentia::standard::Trimmer> regTrimmer;
    AlgorithmFactory::Registrar<BarkBands, essentia::standard::BarkBands> regBarkBands;
    AlgorithmFactory::Registrar<MonoMixer, essentia::standard::MonoMixer> regMonoMixer;
    AlgorithmFactory::Registrar<ChordsDescriptors, essentia::standard::ChordsDescriptors> regChordsDescriptors;
    AlgorithmFactory::Registrar<OnsetDetection, essentia::standard::OnsetDetection> regOnsetDetection;
    AlgorithmFactory::Registrar<LPC, essentia::standard::LPC> regLPC;
    AlgorithmFactory::Registrar<LogAttackTime, essentia::standard::LogAttackTime> regLogAttackTime;
    AlgorithmFactory::Registrar<SuperFluxExtractor, essentia::standard::SuperFluxExtractor> regSuperFluxExtractor;
    AlgorithmFactory::Registrar<Slicer, essentia::standard::Slicer> regSlicer;
    AlgorithmFactory::Registrar<SpectralContrast, essentia::standard::SpectralContrast> regSpectralContrast;
    AlgorithmFactory::Registrar<SBic, essentia::standard::SBic> regSBic;
    AlgorithmFactory::Registrar<OnsetRate, essentia::standard::OnsetRate> regOnsetRate;
    AlgorithmFactory::Registrar<StereoMuxer, essentia::standard::StereoMuxer> regStereoMuxer;
    AlgorithmFactory::Registrar<Centroid, essentia::standard::Centroid> regCentroid;
    AlgorithmFactory::Registrar<TriangularBands, essentia::standard::TriangularBands> regTriangularBands;
    AlgorithmFactory::Registrar<StereoTrimmer, essentia::standard::StereoTrimmer> regStereoTrimmer;
    AlgorithmFactory::Registrar<Scale, essentia::standard::Scale> regScale;
    AlgorithmFactory::Registrar<VectorRealAccumulator> regVectorRealAccumulator;
    AlgorithmFactory::Registrar<PoolAggregator, essentia::standard::PoolAggregator> regPoolAggregator;
    AlgorithmFactory::Registrar<LowLevelSpectralExtractor, essentia::standard::LowLevelSpectralExtractor> regLowLevelSpectralExtractor;
    AlgorithmFactory::Registrar<IDCT, essentia::standard::IDCT> regIDCT;
    AlgorithmFactory::Registrar<OverlapAdd, essentia::standard::OverlapAdd> regOverlapAdd;
    AlgorithmFactory::Registrar<EffectiveDuration, essentia::standard::EffectiveDuration> regEffectiveDuration;
    AlgorithmFactory::Registrar<Panning, essentia::standard::Panning> regPanning;
    AlgorithmFactory::Registrar<LowLevelSpectralEqloudExtractor, essentia::standard::LowLevelSpectralEqloudExtractor> regLowLevelSpectralEqloudExtractor;
    AlgorithmFactory::Registrar<StartStopSilence, essentia::standard::StartStopSilence> regStartStopSilence;
    AlgorithmFactory::Registrar<StrongDecay, essentia::standard::StrongDecay> regStrongDecay;
    AlgorithmFactory::Registrar<SingleBeatLoudness, essentia::standard::SingleBeatLoudness> regSingleBeatLoudness;
    AlgorithmFactory::Registrar<Crest, essentia::standard::Crest> regCrest;
    AlgorithmFactory::Registrar<FileOutputProxy> regFileOutputProxy;
    AlgorithmFactory::Registrar<Key, essentia::standard::Key> regKey;
    AlgorithmFactory::Registrar<EqualLoudness, essentia::standard::EqualLoudness> regEqualLoudness;
    AlgorithmFactory::Registrar<WarpedAutoCorrelation, essentia::standard::WarpedAutoCorrelation> regWarpedAutoCorrelation;
    AlgorithmFactory::Registrar<ReplayGain, essentia::standard::ReplayGain> regReplayGain;
    AlgorithmFactory::Registrar<RealAccumulator> regRealAccumulator;
    AlgorithmFactory::Registrar<NSGConstantQStreaming> regNSGConstantQStreaming;
    AlgorithmFactory::Registrar<BeatTrackerDegara, essentia::standard::BeatTrackerDegara> regBeatTrackerDegara;
    AlgorithmFactory::Registrar<GFCC, essentia::standard::GFCC> regGFCC;
    AlgorithmFactory::Registrar<OnsetDetectionGlobal, essentia::standard::OnsetDetectionGlobal> regOnsetDetectionGlobal;
    AlgorithmFactory::Registrar<SpectralCentroidTime, essentia::standard::SpectralCentroidTime> regSpectralCentroidTime;
    AlgorithmFactory::Registrar<EnergyBandRatio, essentia::standard::EnergyBandRatio> regEnergyBandRatio;
    AlgorithmFactory::Registrar<IFFTA, essentia::standard::IFFTA> regIFFTA;
    AlgorithmFactory::Registrar<FlatnessDB, essentia::standard::FlatnessDB> regFlatnessDB;
    AlgorithmFactory::Registrar<AudioOnsetsMarker, essentia::standard::AudioOnsetsMarker> regAudioOnsetsMarker;
    AlgorithmFactory::Registrar<KeyExtractor, essentia::standard::KeyExtractor> regKeyExtractor;
    AlgorithmFactory::Registrar<Flatness, essentia::standard::Flatness> regFlatness;
    AlgorithmFactory::Registrar<TempoTapDegara, essentia::standard::TempoTapDegara> regTempoTapDegara;
    AlgorithmFactory::Registrar<BpmHistogram, essentia::standard::BpmHistogram> regBpmHistogram;
    AlgorithmFactory::Registrar<Clipper, essentia::standard::Clipper> regClipper;
    AlgorithmFactory::Registrar<Derivative, essentia::standard::Derivative> regDerivative;
    AlgorithmFactory::Registrar<NoiseAdder, essentia::standard::NoiseAdder> regNoiseAdder;
    AlgorithmFactory::Registrar<PercivalBpmEstimator, essentia::standard::PercivalBpmEstimator> regPercivalBpmEstimator;
    AlgorithmFactory::Registrar<SpectralComplexity, essentia::standard::SpectralComplexity> regSpectralComplexity;
    AlgorithmFactory::Registrar<HarmonicBpm, essentia::standard::HarmonicBpm> regHarmonicBpm;
    AlgorithmFactory::Registrar<Danceability, essentia::standard::Danceability> regDanceability;
    AlgorithmFactory::Registrar<TuningFrequency, essentia::standard::TuningFrequency> regTuningFrequency;
    AlgorithmFactory::Registrar<HFC, essentia::standard::HFC> regHFC;
    AlgorithmFactory::Registrar<RMS, essentia::standard::RMS> regRMS;
    AlgorithmFactory::Registrar<LoudnessEBUR128, essentia::standard::LoudnessEBUR128> regLoudnessEBUR128;
    AlgorithmFactory::Registrar<BPF, essentia::standard::BPF> regBPF;
    AlgorithmFactory::Registrar<Spectrum, essentia::standard::Spectrum> regSpectrum;
    AlgorithmFactory::Registrar<Windowing, essentia::standard::Windowing> regWindowing;
    AlgorithmFactory::Registrar<RhythmExtractor, essentia::standard::RhythmExtractor> regRhythmExtractor;
    AlgorithmFactory::Registrar<Mean, essentia::standard::Mean> regMean;
    AlgorithmFactory::Registrar<Onsets, essentia::standard::Onsets> regOnsets;
    AlgorithmFactory::Registrar<FrameCutter, essentia::standard::FrameCutter> regFrameCutter;
    AlgorithmFactory::Registrar<FrequencyBands, essentia::standard::FrequencyBands> regFrequencyBands;
    AlgorithmFactory::Registrar<EnergyBand, essentia::standard::EnergyBand> regEnergyBand;
    AlgorithmFactory::Registrar<BpmHistogramDescriptors, essentia::standard::BpmHistogramDescriptors> regBpmHistogramDescriptors;
    AlgorithmFactory::Registrar<MovingAverage, essentia::standard::MovingAverage> regMovingAverage;
    AlgorithmFactory::Registrar<Magnitude, essentia::standard::Magnitude> regMagnitude;
    AlgorithmFactory::Registrar<DistributionShape, essentia::standard::DistributionShape> regDistributionShape;
    AlgorithmFactory::Registrar<MinToTotal, essentia::standard::MinToTotal> regMinToTotal;
    AlgorithmFactory::Registrar<CentralMoments, essentia::standard::CentralMoments> regCentralMoments;
    AlgorithmFactory::Registrar<Spline, essentia::standard::Spline> regSpline;
    AlgorithmFactory::Registrar<LevelExtractor, essentia::standard::LevelExtractor> regLevelExtractor;
    AlgorithmFactory::Registrar<Decrease, essentia::standard::Decrease> regDecrease;
    AlgorithmFactory::Registrar<PeakDetection, essentia::standard::PeakDetection> regPeakDetection;
    AlgorithmFactory::Registrar<FFTA, essentia::standard::FFTA> regFFTA;
    AlgorithmFactory::Registrar<Dissonance, essentia::standard::Dissonance> regDissonance;
    AlgorithmFactory::Registrar<Multiplexer, essentia::standard::Multiplexer> regMultiplexer;
    AlgorithmFactory::Registrar<BinaryOperatorStream, essentia::standard::BinaryOperatorStream> regBinaryOperatorStream;
    AlgorithmFactory::Registrar<HighResolutionFeatures, essentia::standard::HighResolutionFeatures> regHighResolutionFeatures;
    AlgorithmFactory::Registrar<LoopBpmConfidence, essentia::standard::LoopBpmConfidence> regLoopBpmConfidence;
    AlgorithmFactory::Registrar<CartesianToPolar, essentia::standard::CartesianToPolar> regCartesianToPolar;
    AlgorithmFactory::Registrar<ZeroCrossingRate, essentia::standard::ZeroCrossingRate> regZeroCrossingRate;
    AlgorithmFactory::Registrar<TempoTapMaxAgreement, essentia::standard::TempoTapMaxAgreement> regTempoTapMaxAgreement;
    AlgorithmFactory::Registrar<BeatTrackerMultiFeature, essentia::standard::BeatTrackerMultiFeature> regBeatTrackerMultiFeature;
    AlgorithmFactory::Registrar<RollOff, essentia::standard::RollOff> regRollOff;
    AlgorithmFactory::Registrar<BpmRubato, essentia::standard::BpmRubato> regBpmRubato;
    AlgorithmFactory::Registrar<LoopBpmEstimator, essentia::standard::LoopBpmEstimator> regLoopBpmEstimator;
    AlgorithmFactory::Registrar<TonalExtractor, essentia::standard::TonalExtractor> regTonalExtractor;
    AlgorithmFactory::Registrar<IIR, essentia::standard::IIR> regIIR;
    AlgorithmFactory::Registrar<SuperFluxPeaks, essentia::standard::SuperFluxPeaks> regSuperFluxPeaks;
    AlgorithmFactory::Registrar<LoudnessEBUR128Filter> regLoudnessEBUR128Filter;
    AlgorithmFactory::Registrar<Envelope, essentia::standard::Envelope> regEnvelope;
    AlgorithmFactory::Registrar<SilenceRate, essentia::standard::SilenceRate> regSilenceRate;
    AlgorithmFactory::Registrar<TCToTotal, essentia::standard::TCToTotal> regTCToTotal;
    AlgorithmFactory::Registrar<PitchSalience, essentia::standard::PitchSalience> regPitchSalience;
    AlgorithmFactory::Registrar<UnaryOperator, essentia::standard::UnaryOperator> regUnaryOperator;
    AlgorithmFactory::Registrar<Entropy, essentia::standard::Entropy> regEntropy;
    AlgorithmFactory::Registrar<RhythmTransform, essentia::standard::RhythmTransform> regRhythmTransform;
    AlgorithmFactory::Registrar<BFCC, essentia::standard::BFCC> regBFCC;
    AlgorithmFactory::Registrar<Flux, essentia::standard::Flux> regFlux;
    AlgorithmFactory::Registrar<CrossCorrelation, essentia::standard::CrossCorrelation> regCrossCorrelation;
    AlgorithmFactory::Registrar<FadeDetection, essentia::standard::FadeDetection> regFadeDetection;
    AlgorithmFactory::Registrar<TempoScaleBands, essentia::standard::TempoScaleBands> regTempoScaleBands;
    AlgorithmFactory::Registrar<SingleGaussian, essentia::standard::SingleGaussian> regSingleGaussian;
    AlgorithmFactory::Registrar<DynamicComplexity, essentia::standard::DynamicComplexity> regDynamicComplexity;
    AlgorithmFactory::Registrar<UnaryOperatorStream, essentia::standard::UnaryOperatorStream> regUnaryOperatorStream;
    AlgorithmFactory::Registrar<ChordsDetection, essentia::standard::ChordsDetection> regChordsDetection;
    AlgorithmFactory::Registrar<MFCC, essentia::standard::MFCC> regMFCC;
    AlgorithmFactory::Registrar<FrameToReal, essentia::standard::FrameToReal> regFrameToReal;
    AlgorithmFactory::Registrar<PowerSpectrum, essentia::standard::PowerSpectrum> regPowerSpectrum;
    AlgorithmFactory::Registrar<Duration, essentia::standard::Duration> regDuration;
    AlgorithmFactory::Registrar<CubicSpline, essentia::standard::CubicSpline> regCubicSpline;
    AlgorithmFactory::Registrar<AudioLoader, essentia::standard::AudioLoader> regAudioLoader;
    AlgorithmFactory::Registrar<EasyLoader, essentia::standard::EasyLoader> regEasyLoader;
    AlgorithmFactory::Registrar<EqloudLoader, essentia::standard::EqloudLoader> regEqloudLoader;
    AlgorithmFactory::Registrar<MonoLoader, essentia::standard::MonoLoader> regMonoLoader;
    AlgorithmFactory::Registrar<MetadataReader, essentia::standard::MetadataReader> regMetaDataReader;
    AlgorithmFactory::Registrar<Resample, essentia::standard::Resample> regResample;
}}}
