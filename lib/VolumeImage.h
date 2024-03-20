#pragma once

#include "vtkAutoInit.h"
#include "vtkColorSeries.h"

#include <itkImage.h>
#include <vtkAssembly.h>
#include <vtkCutter.h>
#include <vtkGenericClip.h>
#include <vtkImageData.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkLookupTable.h>
#include <vtkNamedColors.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkTransformFilter.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)

#include "Data3d.h"

class vtkBox;
class vtkImplicitBoolean;
class vtkClipPolyData;
class vtkDataSetMapper;
class vtkImageReslice;
class vtkExtractVOI;
class vtkImageSlice;

namespace ultrast {
namespace infrastructure {
namespace utility {

enum class VolumeImageType : int
{
    UNKNOWN = 0,
    ULTRASOUND,
    CT,
    MRI,
};

class VolumeImage : public Data3d, public FileData
{
    Q_OBJECT
public:
    VolumeImage(QString filePath, QString name = QString(),
        const VolumeImageType& volumeImage = VolumeImageType::UNKNOWN);
    VolumeImage(QString filePath, QString name, std::vector<std::string> files,
        const VolumeImageType& volumeImage = VolumeImageType::UNKNOWN);
    VolumeImage(int id);
    virtual ~VolumeImage();
    void setVisible(bool visible) override;
    void getBounds(double (&bounds)[6]) override;
    void setOpacity(double opacity) override;
    void setFilePath(const QString& filePath) override;
    void setLoadingOrder(int loadingOrder);
    void setWindow(bool increase);
    void setLevel(bool increase);
    void setAutoContrast(bool incresae);
    //get methods
    vtkSmartPointer<vtkImageData> getImageData() const { return m_imageData; }
    vtkSmartPointer<vtkImageSlice> getImageSlice(QString viewerId) const;
    vtkSmartPointer<vtkAssembly> getAssembly(QString viewerId) const;
    vtkImageSlice* getFlexibleImageSlice(const QString& viewerId);
    vtkSmartPointer<vtkImageSlice>* getStereoImageSlice(
        const QString& viewerId);
    int getLoadingOrder() const { return m_loadingOrder; };
    vtkColorSeries::ColorSchemes getCurrentColorScheme()
    {
        return m_currentColorScheme;
    };
    /**
    * @brief get original voxel data range
    *
    * @param volume image contrast max range
    */
    void getVoxelDataRange(double range[2]) const;
    /**
    * @brief set different contrast steps for different images
    *
    */
    void setContrastSingleStep();
    /**
    * @brief store image Settings for completed contrast
    *
    * @param volume image current contrast range
    */
    void storeContrastRange(
        std::shared_ptr<std::vector<std::pair<double, double>>> cRange);
    void setContrastStep(bool increase);
    /**
    * @brief get contrast single step
    *
    */
    double getContrastSingleStep() { return m_contrastSingleStep; };
    /**
    * @brief current min and max value, representing Window Level
    *
    * @param volume image current contrast range
    */
    void getLookupTableRange(double range[2]) const;
    void setLookupTableRange(double min, double max);
    /**
    * @brief create an automatic contrast gradient meter
    *
    */
    void buildContrastList();
    /**
    * @brief Get percentile records
    *
    */
    std::shared_ptr<std::vector<std::pair<double, double>>> getContrastRange()
    {
        return m_percentileRecords;
    }
    /**
    * @brief set and switch color maps
    *
    * @param set times
    */
    void setColorMaps(int index);
    void switchColorMaps(bool next);
    void switchColorMaps(const vtkColorSeries::ColorSchemes& color);
    //functions used for database handling
    virtual const QJsonDocument toQJsonDocument() const override;
    virtual void fromQJsonDocument(QJsonDocument& document) override;
    const VolumeImageType& getVolumeImageType() { return m_volumeImageType; };
    /**
    * @brief static method to convert non-nrrd typed files into nrrd type and save to local disk with my own method
    *
    */
    static bool convertNonNrrdFileToNrrd(
        std::vector<std::string>& inputNonNrrdFiles,
        std::string saveLocalNrrdFile, std::string saveType);
    static bool convertNonNrrdFileToNrrdITK(
        std::vector<std::string>& inputNonNrrdFiles,
        std::string saveLocalNrrdFile, std::string saveType);
    /**
    * @brief Set the main image to be used for image registration
    *
    */
    void setMain(bool main);
    bool isMain() { return m_main; }

signals:
    void mainUpdated(bool);

public slots:
    void setStereoSlicePosition(const unit::Point& coord);

protected:
    using InputPixelType = short;
    //output pixel type is float, this is because registration now just supports float type
    using OutputPixelType = float;
    void initialize();
    virtual void setUpForSliceViewer(ViewerInfo info) override;
    virtual void setUpForStereoViewer(ViewerInfo info) override;
    virtual void setUpFlexibleSliceViewer(const QString& viewerId,
        const FlexibleSliceDataInfo& info) override;
    void readOneFile();
    void readFromFiles();
    const bool readWithItk();
    /**
    * @brief convert vtkImageData to nrrd type and save to local disk with itk official api
    *
    */
    void saveToNrrdFileWithItkApi(
        const itk::Image<short, 3>::Pointer& itkImagePointer,
        std::string output);
    const bool fileExists(const QString& filePath);

    /**
    * @brief convert vtkImageData to nrrd type and save to local disk with my own method
    *
    */
    void saveToNrrdFileByLocalApi(
        const itk::Image<short, 3>::Pointer& itkImagePointer,
        std::string output, std::string saveType);

    bool m_main = false;
    bool m_oneFile = true;
    std::vector<std::string> m_filesPath;
    double m_opacity;
    int m_loadingOrder = -1;
    vtkSmartPointer<vtkImageData> m_imageData;
    VolumeImageType m_volumeImageType;
    unit::Point m_userDefinedcenter;

    /********************************************/
    std::vector<double> m_voxelDataRange;
    double m_contrastSingleStep;
    double m_contrastStepPercent = 0.05;
    std::weak_ptr<std::vector<std::pair<double, double>>> m_contrastRange;
    //Colormap:
    std::vector<vtkColorSeries::ColorSchemes> m_schemesContainer;
    vtkColorSeries::ColorSchemes m_currentColorScheme;

    std::vector<double> m_thresholdOfBlackPixels { 0.6, 0.6, 0.6 };
    //Percentils - range:
    std::shared_ptr<std::vector<std::pair<double, double>>> m_percentileRecords;
    //each viewer needs their own mapper etc. vtk doesn't allow sharing
    //vtkImageSlice is where visibility is controlled so for centralized
    //control, these vars should be stored together here
    //for SliceViewer
    struct SliceComponents
    {
        vtkSmartPointer<vtkImageResliceMapper> m_resliceMapper;
        vtkSmartPointer<vtkLookupTable> m_lookupTable;
        vtkSmartPointer<vtkImageSlice> m_imageSlice;
    };
    std::vector<SliceComponents> m_sliceComponents;
    std::map<QString, SliceComponents> m_sliceComponentsMap;
    //for StereoViewer
    struct StereoComponents
    {
        //for the slice
        vtkSmartPointer<vtkImageResliceMapper> m_resliceMapper[3];
        vtkSmartPointer<vtkLookupTable> m_lookupTable[3];
        vtkSmartPointer<vtkImageSlice> m_imageSlice[3];
        //for the border
        vtkSmartPointer<vtkCutter> m_cutter[3];
        vtkSmartPointer<vtkPolyDataMapper> m_mapper[3];
        vtkSmartPointer<vtkActor> m_actor[3];

        vtkSmartPointer<vtkAssembly> m_assembly;
    };
    std::vector<StereoComponents> m_stereoComponents;
    std::map<QString, StereoComponents> m_stereoComponentsMap;
    struct FlexibleSliceComponents
    {
        vtkSmartPointer<vtkImageReslice> m_imageReslice;
        vtkSmartPointer<vtkImageSliceMapper> m_sliceMapper;
        vtkSmartPointer<vtkImageSlice> m_imageSlice;
        vtkSmartPointer<vtkImageReslice> m_imageFlip;
    };
    std::vector<FlexibleSliceComponents> m_flexibleSliceComponents;
    std::map<QString, FlexibleSliceComponents> m_flexibleSliceComponentsMap;
};

} // namespace utility
} // namespace interface
}; // namespace ultrast