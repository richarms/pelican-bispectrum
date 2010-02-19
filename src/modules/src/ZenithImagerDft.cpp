#include "modules/ZenithImagerDft.h"
#include "utility/constants.h"
#include "utility/pelicanTimer.h"
#ifdef USE_MKL
    #include <mkl_cblas.h>
#else
    #include "cblas.h"
#endif
#include <QString>
#include <QStringList>
#include <iostream>

#include "utility/memCheck.h"


namespace pelican {

/**
 * @details
 * Module constructor.
 */
ZenithImagerDft::ZenithImagerDft(const ConfigNode& config)
    : AbstractModule(config)
{
    // Register which data blobs are needed by the module
    addStreamData("VisibilityData");
    addServiceData("AntennaPositions");
    addGeneratedData("ImageData");

    // Initialise local data pointers
    _vis = NULL;
    _antPos = NULL;
    _image = NULL;

    // Extract configuration from the xml configuration node.
    _getConfiguration(config);

    // Generate image pixel coordinate vectors based on the configuration.
    _coordL.resize(_sizeL);
    _coordM.resize(_sizeM);
    _calculateImageCoords(_cellsizeL, _sizeL, &_coordL[0]);
    _calculateImageCoords(_cellsizeM, _sizeM, &_coordM[0]);

}


/**
 * @details
 * Module destructor.
 */
ZenithImagerDft::~ZenithImagerDft()
{
}


/**
 * @details
 * Sets the image size to the specified values and calculates
 * image coordinate vectors.
 */
void ZenithImagerDft::setSize(const unsigned& sizeL, const unsigned& sizeM)
{
    _sizeL = sizeL;
    _sizeM = sizeM;
    _coordL.resize(_sizeL);
    _coordM.resize(_sizeM);
    if (_fullSky) _setCellsizeFullSky();
    _calculateImageCoords(_cellsizeL, _sizeL, &_coordL[0]);
    _calculateImageCoords(_cellsizeM, _sizeM, &_coordM[0]);
}


/**
 * @details
 * Sets the image cellsize to the specified values and calculates
 * image coordinate vectors.
 */
void ZenithImagerDft::setCellsize(const double& cellsizeL,
        const double& cellsizeM)
{
    _fullSky = false;
    _cellsizeL = cellsizeL;
    _cellsizeM = cellsizeM;
    _calculateImageCoords(_cellsizeL, _sizeL, &_coordL[0]);
    _calculateImageCoords(_cellsizeM, _sizeM, &_coordM[0]);
}


/**
 * @details
 * Sets the image size and cellsize to the specified values and
 * calculate image coordinate vectors.
 */
void ZenithImagerDft::setDimensions(const unsigned& sizeL, const unsigned& sizeM,
                const double& cellsizeL, const double& cellsizeM)
{
    _fullSky = false;
    _cellsizeL = cellsizeL;
    _cellsizeM = cellsizeM;
    _sizeL = sizeL;
    _sizeM = sizeM;
    _coordL.resize(_sizeL);
    _coordM.resize(_sizeM);
    _calculateImageCoords(_cellsizeL, _sizeL, &_coordL[0]);
    _calculateImageCoords(_cellsizeM, _sizeM, &_coordM[0]);
}


/**
 * @details
 * Sets the imager to make a full sky image.
 */
void ZenithImagerDft::setFullSky()
{
    _fullSky = true;
    _setCellsizeFullSky();
    _calculateImageCoords(_cellsizeL, _sizeL, &_coordL[0]);
    _calculateImageCoords(_cellsizeM, _sizeM, &_coordM[0]);
}


/**
 * @details
 * Method called by the pipeline to create images of the visibility data.
 */
void ZenithImagerDft::run(QHash<QString, DataBlob*>& data)
{
    _fetchDataBlobs(data);

    unsigned nAnt = _antPos->nAntennas();
    unsigned nPol = _polarisation == POL_BOTH ? 2 : 1;
    unsigned nChan = _channels.size();
    double frequencyInc = _maxFrequency / static_cast<double>(_nChannels);

    /// Assign memory for the image cube (only resizes if needed)
    _image->resize(_sizeL, _sizeM, nChan, nPol);

    /// Loop over selected channels and polarisations to make images
    for (unsigned c = 0; c < nChan; c++) {

        // The channel list channel id selection
        unsigned channel = _channels[c];

//        if (channel > _freqList->nChannels())
//            throw QString("Selected channel out of range.");
//
        //_freqList->at(channel);
        double frequency = frequencyInc * channel;

        for (unsigned p = 0; p < nPol; p++) {

            unsigned pol = p;
            if (nPol == 1) pol = _polarisation;

            // Get pointers to the visibility data and image for the selected
            // channel and polarisation
            std::cout << channel << " " << pol << " " << frequency <<  std::endl;
            complex_t* vis = _vis->ptr(channel, pol);
            real_t* image = _image->ptr(p, c);

            _makeImageDft(nAnt, _antPos->xPtr(), _antPos->yPtr(), vis, frequency,
                    _sizeL, _sizeM, &_coordL[0], &_coordM[0], image);
        }
    }

    _image->findAmpRange(); // this only works for an image of one channel

//     check nan and inf
//    if (_image->max() != _image->max() && !(_image->max() != image->max())


    std::cout << _image->min() << " " << _image->max() << std::endl;

}


/**
 * @details
 * Parse and extract options from the configuration XML node.
 * Default options are set if configuration options are not found.
 *
 * @param[in]   config  Configuration options XML node.
 */
void ZenithImagerDft::_getConfiguration(const ConfigNode &config)
{
    _sizeL = config.getOption("size", "l", "128").toUInt();
    _sizeM = config.getOption("size", "m", "128").toUInt();
    _fullSky = config.getOption("fullSky", "value", "true") == "true" ? true : false;

    // Full sky = set l and m cellsize for a full image ignoring other settings.
    if (_fullSky) {
        _setCellsizeFullSky();
    }
    else {
        _cellsizeL = config.getOption("cellsize", "l", "10.0").toDouble();
        _cellsizeM = config.getOption("cellsize", "m", "10.0").toDouble();
    }

    // Get the polarisation selection.
    QString pol = config.getOption("polarisation", "value", "x").toLower();
    if (pol == "x") _polarisation = POL_X;
    else if (pol == "y") _polarisation = POL_Y;
    else if (pol == "both") _polarisation = POL_BOTH;

    // Get the channels to image.
    QString chan = config.getOptionText("channels", "0");
    QStringList chanList = chan.split(",", QString::SkipEmptyParts);
    _channels.resize(chanList.size());
    for (int c = 0; c < chanList.size(); c++) {
        _channels[c] = chanList.at(c).toUInt();
    }

    _nChannels = config.getOption("frequencies", "number", "512").toUInt();
    _maxFrequency = config.getOption("frequencies", "max", "1.0e8").toDouble();
}


/**
 * @details
 * Generate an image coordinate vector (\p coords) based on the pixel
 * separation in arcsec (\p cellsize) and number of pixels (\p nPixels).
 *
 * Image coordinates are for either the l (x) or m (y) depending on which
 * values are passed as input arguments.
 *
 * The pixel vector is resized to the number of pixels by the function.
 *
 * @param[in]  cellsize  Image pixel separation in arcseconds.
 * @param[in]  nPixels   Number of image pixels along the axis.
 * @param[out] coords    Reference to a vector of image coordinates.
 */
void ZenithImagerDft::_calculateImageCoords(const double& cellsize,
        const unsigned& nPixels, real_t* coords)
{
    double delta = cellsize * math::asec2rad;
    int centre = nPixels / 2;
    for (int i = 0; i < static_cast<int>(nPixels); i++) {
        coords[i] = static_cast<double>(i - centre) * delta;
//        std::cout << coords[i] << std::endl;
    }
}


/**
 * @details
 * Fetches named data blobs from the data blob hash into local local varibles.
 */
void ZenithImagerDft::_fetchDataBlobs(QHash<QString, DataBlob*>& data)
{
    _vis = static_cast<VisibilityData*>(data["VisibilityData"]);
    _antPos = static_cast<AntennaPositions*>(data["AntennaPositions"]);
    _image = static_cast<ImageData*>(data["ImageData"]);

    if (!_vis) throw QString("ZenithImagerDft: VisibilityData blob missing");
    if (!_antPos) throw QString("ZenithImagerDft: AntennaPositiosn blob missing");
    if (!_image) throw QString("ZenithImagerDft: ImageData blob missing");

    if (_vis->nAntennas() == 0) throw QString("Empty data blob: VisibilityData");
    if (_antPos->nAntennas() == 0) throw QString("Empty data blob: AntennaPositions");

    if (_vis->nAntennas() != _antPos->nAntennas())
        throw QString("ZenithImagerDft: data blob dimension mismatch");

//    for (unsigned i = 0; _vis->nEntries(); i++) {
//        std::cout << (*_vis)[i] << std::endl;
//    }


}


/**
 * @details
 * Calculates a matrix of complex weights for forming an image by
 * 2D DFT.
 */
void ZenithImagerDft::_calculateWeights(const unsigned& nAnt, real_t* antPos,
        const double& frequency, const unsigned& nCoords,
        real_t* imageCoord, complex_t* weights)
{
    double k = (math::twoPi * frequency) / phy::c;
//    std::cout << frequency << std::endl;
//    std::cout << math::twoPi << std::endl;
//    std::cout << phy::c << std::endl;
//    std::cout << "k = " << k << std::endl;
    for (unsigned i = 0; i < nCoords; i++) {
        unsigned index = i * nAnt;
        double arg1 = k * imageCoord[i];
        for (unsigned a = 0; a < nAnt; a++) {
            double arg2 = arg1 * antPos[a];
            weights[index + a] = complex_t(cos(arg2), sin(arg2));
//            std::cout << weights[index + a] << std::endl;
        }
    }
}


/**
 * @details
 * Perform a discrete Fourier transform to form an image from the visibility data.
 */
void ZenithImagerDft::_makeImageDft(const unsigned& nAnt, real_t* antPosX,
        real_t* antPosY, complex_t* vis, const double& frequency,
        const unsigned& nL, const unsigned& nM,
        real_t* coordsL, real_t* coordsM, real_t *image)
{
    _weightsXL.resize(nAnt * nL);
    _weightsYM.resize(nAnt * nM);
    _calculateWeights(nAnt, antPosX, frequency, nL, coordsL, &_weightsXL[0]);
    _calculateWeights(nAnt, antPosY, frequency, nM, coordsM, &_weightsYM[0]);

    _weights.resize(nAnt);
    _temp.resize(nAnt);


//    for (unsigned m = 0; m < nM; m++) {
//        for (unsigned l = 0; l < nL; l++) {
//            for (unsigned j = 0; j < nAnt; j++) {
//                for (unsigned i = 0; i < nAnt; i++) {
//                    image[indexM + l] +=
//
//                }
//            }
//        }
//    }




#ifdef USE_BLAS
    real_t alpha[2] = {1.0, 0.0};
    real_t beta[2]  = {0.0, 0.0};
#endif

    for (unsigned m = 0; m < nM; m++) {
        complex_t *weightsYM = &_weightsYM[m * nAnt];
        unsigned indexM = m * nL;

        for (unsigned l = 0; l < nL; l++) {
            complex_t * weightsXL = &_weightsXL[l * nAnt];
            _multWeights(nAnt, weightsXL, weightsYM, &_weights[0]);

#ifdef USE_BLAS
            cblas_zgemv(CblasRowMajor, CblasNoTrans, nAnt, nAnt, alpha, vis,
                    nAnt, &_weights[0], 1, beta, &_temp[0], 1);
#else
            _multMatrixVector(nAnt, vis, &_weights[0], &_temp[0]);
#endif

            /// use some sort of cblas_cdot to replace this call.
            image[indexM + l] = _vectorDotConj(nAnt, &_temp[0], &_weights[0]).real();
//            std::cout << m << " " << l << " " << image[indexM + l] << std::endl;
        }
    }
}


/**
 * @details
 * Element wise multiplication of two weights vectors.
 * - need to find a blas function to do this...
 */
void ZenithImagerDft::_multWeights(const unsigned& nAnt, complex_t* weightsXL,
        complex_t *weightsYM, complex_t *weights)
{
    for (unsigned i = 0; i < nAnt; i++) {
        weights[i] = weightsXL[i] * weightsYM[i];
//        std::cout << weights[i] << std::endl;
    }
}


/**
 * @details
 * Performs matrix vector multiply of a matrix of visibility amplitudes
 * by a vector of complex dft weights
 */
void ZenithImagerDft::_multMatrixVector(const unsigned& nAnt,
        complex_t* visMatrix, complex_t *weights, complex_t* result)
{
    for (unsigned j = 0; j < nAnt; j++) {
        unsigned indexJ = j * nAnt;
        for (unsigned i = 0; i < nAnt; i++) {
            result[j] += visMatrix[indexJ + i] * weights[i];
//            std::cout << visMatrix[indexJ + i] << " " << weights[i] << " - " << result[j] << std::endl;
        }
    }
}


/**
 * @details
 * Vector dot product
 */
complex_t ZenithImagerDft::_vectorDotConj(const unsigned& n, complex_t* a,
        complex_t* b)
{
    complex_t result = complex_t(0.0, 0.0);
    for (unsigned i = 0; i < n; i++) {
        result += a[i] * conj(b[i]);
    }
    return result;
}



/**
 * @details
 */
void ZenithImagerDft::_cutHemisphere()
{
}


/**
 * @details
 */
void ZenithImagerDft::_setCellsizeFullSky()
{
    _cellsizeL = math::pi / _sizeL * math::rad2asec;
    _cellsizeM = math::pi / _sizeM * math::rad2asec;
}


} // namespace pelican
