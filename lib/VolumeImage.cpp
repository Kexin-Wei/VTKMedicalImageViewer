#include <QDebug>
#include <QFileInfo>

#include <itkCastImageFilter.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMImageIOFactory.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageSeriesReader.h>
#include <itkImageToVTKImageFilter.h>
#include <itkMINCImageIO.h>
#include <itkMINCImageIOFactory.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaImageIO.h>
#include <itkMetaImageIOFactory.h>
#include <itkNiftiImageIO.h>
#include <itkNiftiImageIOFactory.h>
#include <itkNrrdImageIO.h>
#include <itkNrrdImageIOFactory.h>
#include <itkVTKImageToImageFilter.h>

#include <vtkBox.h>
#include <vtkClipPolyData.h>
#include <vtkCubeSource.h>
#include <vtkCutter.h>
#include <vtkDICOMImageReader.h>
#include <vtkDataSet.h>
#include <vtkImageHistogramStatistics.h>
#include <vtkImageProperty.h>
#include <vtkImageReslice.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageWriter.h>
#include <vtkImplicitBoolean.h>
#include <vtkMINCImageReader.h>
#include <vtkMetaImageReader.h>
#include <vtkPlaneCollection.h>

// #include <infrastructure\utility\FileManager.h>
#include <VolumeImage.h>

// using FileManager = ultrast::infrastructure::utility::FileManager;

namespace ultrast {
namespace infrastructure {
namespace utility {

VolumeImage::VolumeImage(QString filePath, QString name,
    const VolumeImageType& volumeImage) :
    Data3d(-1, name, true),
    FileData(filePath),
    m_volumeImageType(volumeImage)
{
    initialize();
}

VolumeImage::VolumeImage(QString filePath, QString name,
    std::vector<std::string> files, const VolumeImageType& volumeImage) :
    Data3d(-1, name, true),
    FileData(
        filePath), // use the first file in the purpose of checking whether the the file is already imported
    m_oneFile(false),
    m_filesPath(files),
    m_volumeImageType(volumeImage)
{
    initialize();
}

VolumeImage::VolumeImage(int id) :
    Data3d(id, QString(), true)
{
    // update fields from database if exists
    // if (id > -1)
    //     update();
    initialize();
}

VolumeImage::~VolumeImage()
{
}

void VolumeImage::setVisible(bool visible)
{
    qDebug() << "VolumeImage - set visible: " << visible;
    BaseData::setVisible(visible);
    for (auto& component : m_sliceComponents)
        component.m_imageSlice->SetVisibility(visible);
    for (auto& component : m_stereoComponents)
    {
        component.m_assembly->SetVisibility(visible);
        for (auto& slice : component.m_imageSlice)
            slice->SetVisibility(visible);
    }
    for (auto& component : m_flexibleSliceComponents)
        component.m_imageSlice->SetVisibility(visible);
}

void VolumeImage::getBounds(double (&bounds)[6])
{
    m_boundsActor->GetBounds(bounds);
}

void VolumeImage::setOpacity(double opacity)
{
    qDebug() << "VolumeImage - set opacity: " << opacity;
    for (auto& component : m_sliceComponents)
        component.m_imageSlice->GetProperty()->SetOpacity(opacity);
    for (auto& component : m_stereoComponents)
        for (auto& slice : component.m_imageSlice)
            slice->GetProperty()->SetOpacity(opacity);
    BaseData::setOpacity(opacity);
}

void VolumeImage::setMain(bool main)
{
    m_main = main;
    // autoSave();
}

void VolumeImage::setFilePath(const QString& filePath)
{
    FileData::setFilePath(filePath);
    readOneFile();
    // autoSave();
}

void VolumeImage::setLoadingOrder(int loadingOrder)
{
    m_loadingOrder = loadingOrder;
    qDebug() << "VolumeImage - set loading order to: " << m_loadingOrder;
    // Logger::trace(
    //     QString("VolumeImage - set loading order to: %1").arg(m_loadingOrder));
    for (auto& component : m_sliceComponents)
        component.m_imageSlice->GetProperty()->SetLayerNumber(m_loadingOrder);
    for (auto& component : m_stereoComponents)
        for (auto& slice : component.m_imageSlice)
            slice->GetProperty()->SetLayerNumber(m_loadingOrder);
    // autoSave();
}

vtkSmartPointer<vtkImageSlice> VolumeImage::getImageSlice(
    QString viewerId) const
{
    if (existsInMap(m_sliceComponentsMap, viewerId))
        return m_sliceComponentsMap.at(viewerId).m_imageSlice;
    qDebug() << "VolumeImage - get image slice invalid viewer id: " << viewerId;
    return nullptr;
}

vtkSmartPointer<vtkAssembly> VolumeImage::getAssembly(QString viewerId) const
{
    if (existsInMap(m_stereoComponentsMap, viewerId))
        return m_stereoComponentsMap.at(viewerId).m_assembly;
    qDebug() << "VolumeImage - get assembly invalid viewer id: " << viewerId;
    return nullptr;
}

vtkImageSlice* VolumeImage::getFlexibleImageSlice(const QString& viewerId)
{
    if (existsInMap(m_flexibleSliceComponentsMap, viewerId))
        return m_flexibleSliceComponentsMap.at(viewerId).m_imageSlice;
    qDebug() << "VolumeImage - get flexible image slice invalid viewer id: "
             << viewerId;
    return nullptr;
}

vtkSmartPointer<vtkImageSlice>* VolumeImage::getStereoImageSlice(
    const QString& viewerId)
{
    if (existsInMap(m_stereoComponentsMap, viewerId))
        return m_stereoComponentsMap.at(viewerId).m_imageSlice;
    qDebug() << "VolumeImage - get stereo image invalid viewer id: "
             << viewerId;
    return nullptr;
}

const QJsonDocument VolumeImage::toQJsonDocument() const
{
    auto object = getBaseObject();
    object["filePath"] = m_filePath;
    object["loadingOrder"] = m_loadingOrder;
    object["volumeImageType"] = static_cast<int>(m_volumeImageType);
    object["isMain"] = m_main;
    return QJsonDocument(object);
}

void VolumeImage::fromQJsonDocument(QJsonDocument& document)
{
    if (!document.isNull())
        if (document.isObject())
        {
            QJsonObject object = document.object();
            setBaseObject(object);
            m_filePath = object["filePath"].toString();
            m_loadingOrder = object["loadingOrder"].toInt();
            m_volumeImageType = static_cast<VolumeImageType>(
                object["volumeImageType"].toInt());
            m_main = object["isMain"].toBool();
        }
        else
            qDebug() << "VolumeImage - Document is not an object";
    else
        qDebug() << "VolumeImage - Invalid JSON";
}

void VolumeImage::setStereoSlicePosition(const unit::Point& coord)
{
    double coordArray[3];
    coord.toArray(coordArray);
    for (auto& component : m_stereoComponents)
        for (int i = 0; i < 3; i++)
            component.m_resliceMapper[i]->GetSlicePlane()->SetOrigin(
                coordArray);
}

void VolumeImage::initialize()
{
    m_type = DataType::VOLUME_IMAGE;

    if (!m_oneFile)
    {
        readFromFiles();
    }
    else
    {
        readOneFile();
    }
    m_voxelDataRange.resize(2, 0);
    if (m_imageData)
    {
        vtkNew<vtkCubeSource> boundsCube;
        double* imageBounds = m_imageData->GetBounds();
        boundsCube->SetXLength(imageBounds[1] - imageBounds[0]);
        boundsCube->SetYLength(imageBounds[3] - imageBounds[2]);
        boundsCube->SetZLength(imageBounds[5] - imageBounds[4]);
        boundsCube->SetCenter(m_imageData->GetCenter());

        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(boundsCube->GetOutputPort());
        mapper->Update();
        m_boundsActor = vtkSmartPointer<vtkActor>::New();
        m_boundsActor->SetMapper(mapper);
        m_boundsActor->SetUserTransform(m_worldTransform);

        // Setup Scalar range:
        double range[2];
        m_imageData->GetScalarRange(range);
        m_voxelDataRange[0] = range[0];
        m_voxelDataRange[1] = range[1];
    }

    m_percentileRecords
        = std::make_shared<std::vector<std::pair<double, double>>>(
            std::vector<std::pair<double, double>>(6,
                std::pair<double, double>(0, 0)));

    // color schemes:
    m_currentColorScheme = vtkColorSeries::SPECTRUM;
    m_schemesContainer.push_back(vtkColorSeries::SPECTRUM);
    m_schemesContainer.push_back(vtkColorSeries::WARM);
    m_schemesContainer.push_back(vtkColorSeries::COOL);

    qDebug() << "VolumeImage - Created VolumeImage name: " << m_dataName;
}

void VolumeImage::setUpForSliceViewer(ViewerInfo info)
{
    if (!existsInMap(m_sliceComponentsMap, info.m_viewerId))
    {
        SliceComponents components;
        components.m_resliceMapper
            = vtkSmartPointer<vtkImageResliceMapper>::New();
        components.m_resliceMapper->SetInputData(m_imageData);
        components.m_resliceMapper->SliceFacesCameraOn();
        components.m_resliceMapper->SliceAtFocalPointOn();
        components.m_resliceMapper->BorderOff();

        components.m_lookupTable = vtkSmartPointer<vtkLookupTable>::New();
        components.m_lookupTable->SetRange(m_imageData->GetScalarRange()[0],
            m_imageData->GetScalarRange()[1]);
        components.m_lookupTable->SetValueRange(0.0,
            1.0); // from black to white
        components.m_lookupTable->SetSaturationRange(0.0,
            0.0); // no color saturation
        components.m_lookupTable->SetRampToLinear();
        components.m_lookupTable->Build();

        components.m_imageSlice = vtkSmartPointer<vtkImageSlice>::New();
        components.m_imageSlice->SetMapper(components.m_resliceMapper);
        components.m_imageSlice->GetProperty()->UseLookupTableScalarRangeOn();
        components.m_imageSlice->GetProperty()->SetLookupTable(
            components.m_lookupTable);
        components.m_imageSlice->GetProperty()->SetLayerNumber(m_loadingOrder);
        components.m_imageSlice->SetVisibility(m_visible);
        components.m_imageSlice->SetUserTransform(m_worldTransform);

        m_sliceComponents.push_back(components);
        m_sliceComponentsMap[info.m_viewerId] = components;
        qDebug() << "VolumeImage - set up for viewer id: " << info.m_viewerId;
    }
    else
        qDebug() << "VolumeImage - already set up for viewer id: "
                 << info.m_viewerId;
}

void VolumeImage::setUpForStereoViewer(ViewerInfo info)
{
    if (!existsInMap(m_stereoComponentsMap, info.m_viewerId))
    {
        StereoComponents components;
        components.m_assembly = vtkSmartPointer<vtkAssembly>::New();

        vtkNew<vtkCubeSource> borderBounds;
        double* imageBounds = m_imageData->GetBounds();
        borderBounds->SetXLength(imageBounds[1] - imageBounds[0]);
        borderBounds->SetYLength(imageBounds[3] - imageBounds[2]);
        borderBounds->SetZLength(imageBounds[5] - imageBounds[4]);
        borderBounds->SetCenter(m_imageData->GetCenter());
        // filter is necessary to apply transformation before the vtkcutter
        // else the transform is applied after cutting which is incorrect
        vtkNew<vtkTransformFilter> borderTransform;
        borderTransform->SetInputConnection(borderBounds->GetOutputPort());
        borderTransform->SetTransform(m_worldTransform);
        std::string colorName[3] = { "Green", "Blue", "Red" };

        for (int i = 0; i < 3; i++)
        {
            components.m_resliceMapper[i]
                = vtkSmartPointer<vtkImageResliceMapper>::New();
            components.m_resliceMapper[i]->SetInputData(m_imageData);
            vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
            { // set up plane
                plane->SetOrigin(0, 0, 0);
                double normal[3] = { 1, 0, 0 };
                for (int j = 0; j < 3; j++)
                    normal[j] = i == j ? 1 : 0;
                plane->SetNormal(normal);
            }
            components.m_resliceMapper[i]->SetSlicePlane(plane);
            components.m_resliceMapper[i]->BorderOn();

            components.m_lookupTable[i]
                = vtkSmartPointer<vtkLookupTable>::New();
            components.m_lookupTable[i]->SetRange(
                m_imageData->GetScalarRange()[0],
                m_imageData->GetScalarRange()[1]);
            components.m_lookupTable[i]->SetValueRange(0.0,
                1.0); // from black to white
            components.m_lookupTable[i]->SetSaturationRange(0.0,
                0.0); // no color saturation
            components.m_lookupTable[i]->SetRampToLinear();
            components.m_lookupTable[i]->Build();

            components.m_imageSlice[i] = vtkSmartPointer<vtkImageSlice>::New();
            components.m_imageSlice[i]->SetMapper(
                components.m_resliceMapper[i]);
            components.m_imageSlice[i]
                ->GetProperty()
                ->UseLookupTableScalarRangeOn();
            components.m_imageSlice[i]->GetProperty()->SetLookupTable(
                components.m_lookupTable[i]);
            components.m_imageSlice[i]->SetUserTransform(m_worldTransform);
            components.m_imageSlice[i]->GetProperty()->SetLayerNumber(
                m_loadingOrder);
            // for performance improvement inexchange for image quality
            components.m_imageSlice[i]
                ->GetProperty()
                ->SetInterpolationTypeToNearest();

            components.m_cutter[i] = vtkSmartPointer<vtkCutter>::New();
            components.m_cutter[i]->SetCutFunction(plane);
            components.m_cutter[i]->SetInputConnection(
                borderTransform->GetOutputPort());
            components.m_cutter[i]->Update();

            components.m_mapper[i] = vtkSmartPointer<vtkPolyDataMapper>::New();
            components.m_mapper[i]->SetInputConnection(
                components.m_cutter[i]->GetOutputPort());
            components.m_mapper[i]
                ->SetResolveCoincidentTopologyToPolygonOffset();
            components.m_mapper[i]->Update();

            components.m_actor[i] = vtkSmartPointer<vtkActor>::New();
            vtkNew<vtkNamedColors> colors;
            components.m_actor[i]->GetProperty()->SetColor(
                colors->GetColor3d(colorName[i]).GetData());
            components.m_actor[i]->GetProperty()->SetLineWidth(1);
            components.m_actor[i]->GetProperty()->SetAmbient(1.0);
            components.m_actor[i]->GetProperty()->SetDiffuse(0.0);
            components.m_actor[i]->SetMapper(components.m_mapper[i]);

            components.m_assembly->AddPart(components.m_actor[i]);
            components.m_assembly->SetVisibility(m_visible);
        }

        m_stereoComponents.push_back(components);
        m_stereoComponentsMap[info.m_viewerId] = components;
        qDebug() << "VolumeImage - set up for stereo viewer id: "
                 << info.m_viewerId;
    }
    else
        qDebug() << "VolumeImage - already set up for stereo viewer id: "
                 << info.m_viewerId;
}

void VolumeImage::setUpFlexibleSliceViewer(const QString& viewerId,
    const FlexibleSliceDataInfo& info)
{
    if (!existsInMap(m_flexibleSliceComponentsMap, viewerId))
    {
        FlexibleSliceComponents components;
        double spacing[3] = { 0 };
        m_imageData->GetSpacing(spacing);
        int widthExtent = (info.planeWidth / spacing[0]) - 1;
        int heightExtent = (info.planeHeight / spacing[1]) - 1;

        components.m_imageReslice = vtkSmartPointer<vtkImageReslice>::New();
        components.m_imageReslice->SetInputData(m_imageData);
        components.m_imageReslice->SetOutputDimensionality(2);
        components.m_imageReslice->SetOutputOrigin(0, 0, 0);
        components.m_imageReslice->SetResliceAxes(info.resliceAxes);
        components.m_imageReslice->SetResliceTransform(m_worldTransform);
        components.m_imageReslice->SetOutputExtent(0, widthExtent, 0,
            heightExtent, 0, 0);

        components.m_imageFlip = vtkSmartPointer<vtkImageReslice>::New();
        components.m_imageFlip->SetInputConnection(
            components.m_imageReslice->GetOutputPort());
        components.m_imageFlip->SetResliceAxes(info.flipMatrix);

        components.m_sliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
        components.m_sliceMapper->SetInputConnection(
            components.m_imageFlip->GetOutputPort());

        components.m_imageSlice = vtkSmartPointer<vtkImageSlice>::New();
        components.m_imageSlice->SetMapper(components.m_sliceMapper);

        m_flexibleSliceComponents.push_back(components);
        m_flexibleSliceComponentsMap[viewerId] = components;

        qDebug() << "VolumeImage - set up for flexible slice viewer id: "
                 << viewerId;
    }
    else
        qDebug()
            << "VolumeImage - already set up for flexible slice viewer id: "
            << viewerId;
}

void VolumeImage::readOneFile()
{
    itk::NrrdImageIOFactory::RegisterOneFactory();
    itk::NiftiImageIOFactory::RegisterOneFactory();
    itk::MINCImageIOFactory::RegisterOneFactory();
    itk::MetaImageIOFactory::RegisterOneFactory();
    itk::GDCMImageIOFactory::RegisterOneFactory();

    using InputImageType = itk::Image<InputPixelType, 3>;
    // 1. read the image from local file
    itk::ImageFileReader<InputImageType>::Pointer reader
        = itk::ImageFileReader<InputImageType>::New();

    reader->SetFileName(m_filePath.toLocal8Bit().toStdString());

    try
    {
        reader->Update();
    }
    catch (itk::ExceptionObject& e)
    {
        qDebug() << "Volume Image -- ITK image filter with error: " << e.what();
        return;
    }
    // itkImage to vtkImageData
    typedef itk::ImageToVTKImageFilter<InputImageType> itkToVtkFilter;
    itkToVtkFilter::Pointer itkToVtkConnector = itkToVtkFilter::New();
    itkToVtkConnector->SetInput(reader->GetOutput());
    try
    {
        itkToVtkConnector->Update();
    }
    catch (itk::ExceptionObject& e)
    {
        qDebug() << "Volume Image -- Failed to convert itk to vtk: error "
                 << e.what();
        return;
    }
    const InputImageType::DirectionType& direction
        = reader->GetOutput()->GetDirection();
    // Construct the transformation matrix from the direction cosines
    auto transformation = vtkSmartPointer<vtkMatrix4x4>::New();
    for (unsigned int i = 0; i < 3; ++i)
    {
        for (unsigned int j = 0; j < 3; ++j)
        {
            transformation->SetElement(j, i, direction(i, j));
        }
    }
    auto reslicer = vtkSmartPointer<vtkImageReslice>::New();
    {
        reslicer->SetInputData(itkToVtkConnector->GetOutput());
        reslicer->SetResliceAxes(transformation);
        reslicer->SetInterpolationModeToCubic(); // cubic spline interpolator
        reslicer->SetAutoCropOutput(true);
        reslicer->Update();
    }
    m_imageData = reslicer->GetOutput();
}

void VolumeImage::readFromFiles()
{
    // reading DICOM subseries
    using ImageType = itk::Image<InputPixelType, 3>;
    using ReaderType = itk::ImageSeriesReader<itk::Image<InputPixelType, 3>>;
    using ImageIOType = itk::GDCMImageIO;
    using VtkFilter = itk::ImageToVTKImageFilter<ImageType>;

    auto reader = ReaderType::New();
    auto dicomIO = ImageIOType::New();
    auto vtkFilter = VtkFilter::New();

    reader->SetImageIO(dicomIO);
    reader->SetFileNames(m_filesPath);
    reader->ForceOrthogonalDirectionOff();

    try
    {
        reader->Update();
    }
    catch (itk::ExceptionObject& err)
    {
        qDebug() << "Volume Image -- Failed to read dicom images: "
                 << err.what();
    }

    itk::Image<InputPixelType, 3>::Pointer itkImage = reader->GetOutput();
    // convert itk image to vtk image
    vtkFilter->SetInput(itkImage);
    try
    {
        vtkFilter->Update();
    }
    catch (const itk::ExceptionObject& error)
    {
        qDebug() << "Volume Image -- Failed to convert itk to vtk: "
                 << error.what();
    }

    const ImageType::DirectionType& direction
        = reader->GetOutput()->GetDirection();
    // Construct the transformation matrix from the direction cosines
    auto transformation = vtkSmartPointer<vtkMatrix4x4>::New();
    for (unsigned int i = 0; i < 3; ++i)
    {
        for (unsigned int j = 0; j < 3; ++j)
        {
            transformation->SetElement(j, i, direction(i, j));
        }
    }

    auto reslicer = vtkSmartPointer<vtkImageReslice>::New();
    {
        reslicer->SetInputData(vtkFilter->GetOutput());
        reslicer->SetResliceAxes(transformation);
        reslicer->SetInterpolationModeToCubic();
        reslicer->SetAutoCropOutput(true);
        reslicer->Update();
    }
    m_imageData = reslicer->GetOutput();
}

const bool VolumeImage::readWithItk()
{
    std::string path = m_filePath.toStdString();

    itk::NrrdImageIOFactory::RegisterOneFactory();
    // set up aliasses
    using ImageType = itk::Image<InputPixelType, 3>;
    using ReaderType = itk::ImageFileReader<ImageType>;
    using VtkFilter = itk::ImageToVTKImageFilter<ImageType>;

    auto reader = ReaderType::New();
    {
        reader->SetFileName(path);
        try
        {
            reader->Update();
        }
        catch (itk::ExceptionObject& error)
        {
            qDebug() << "VolumeImage -- Failed to open " << m_filePath
                     << " error: " << error.what();
            return false;
        }
    }

    // itkImage to vtkImageData
    auto vtkFilter = VtkFilter::New();
    {
        vtkFilter->SetInput(reader->GetOutput());
        try
        {
            vtkFilter->Update();
        }
        catch (itk::ExceptionObject& error)
        {
            qDebug() << "VolumeImage -- Failed convert itk to vtk: "
                     << error.what();
            return false;
        }
    }
    const ImageType::DirectionType& direction
        = reader->GetOutput()->GetDirection();

    // Construct the transformation matrix from the direction cosines
    auto transformation = vtkSmartPointer<vtkMatrix4x4>::New();
    for (unsigned int i = 0; i < 3; ++i)
    {
        for (unsigned int j = 0; j < 3; ++j)
        {
            transformation->SetElement(j, i, direction(i, j));
        }
    }

    auto reslicer = vtkSmartPointer<vtkImageReslice>::New();
    {
        reslicer->SetInputData(vtkFilter->GetOutput());
        reslicer->SetResliceAxes(transformation);
        reslicer->SetInterpolationModeToCubic();
        reslicer->SetAutoCropOutput(true);
        reslicer->Update();
    }
    m_imageData = reslicer->GetOutput();
}

void VolumeImage::buildContrastList()
{
    vtkSmartPointer<vtkImageHistogramStatistics> hist_stat
        = vtkSmartPointer<vtkImageHistogramStatistics>::New();
    hist_stat->SetAutomaticBinning(true);
    hist_stat->SetAutoRangeExpansionFactors(0, 0);
    hist_stat->GenerateHistogramImageOff();
    hist_stat->SetInputData(m_imageData);

    double percentileStep = 0.15;
    for (int rangeIdx = 0; rangeIdx < (int)m_percentileRecords->size();
         ++rangeIdx)
    {
        hist_stat->SetAutoRangePercentiles(0.0 + percentileStep * (rangeIdx),
            100.0 - percentileStep * (rangeIdx));
        hist_stat->Update();

        m_percentileRecords->at(rangeIdx).first = hist_stat->GetAutoRange()[0];
        m_percentileRecords->at(rangeIdx).second = hist_stat->GetAutoRange()[1];
    }
}

void VolumeImage::setColorMaps(int index)
{
    if (index == 0)
    {
        for (auto& component : m_sliceComponents)
        {
            component.m_lookupTable->SetNumberOfTableValues(256);
            component.m_lookupTable->SetValueRange(0.0,
                1.0); // from black to white
            component.m_lookupTable->SetAlphaRange(1, 1);
            component.m_lookupTable->SetSaturationRange(0.0,
                0.0); // no color saturation
            component.m_lookupTable->SetHueRange(0.0, 0.0);
            component.m_lookupTable->SetRampToLinear();
            component.m_lookupTable->ForceBuild();
        }
        for (auto& components : m_stereoComponents)
        {
            for (int i = 0; i < 3; i++)
            {
                components.m_lookupTable[i]->SetNumberOfTableValues(256);
                components.m_lookupTable[i]->SetValueRange(0.0,
                    1.0); // from black to white
                components.m_lookupTable[i]->SetAlphaRange(1, 1);
                components.m_lookupTable[i]->SetSaturationRange(0.0,
                    0.0); // no color saturation
                components.m_lookupTable[i]->SetHueRange(0.0, 0.0);
                components.m_lookupTable[i]->SetRampToLinear();
                components.m_lookupTable[i]->ForceBuild();
            }
        }
    }
    else if (index == 1)
    {
        for (auto& component : m_sliceComponents)
        {
            component.m_lookupTable->SetNumberOfTableValues(128);
            component.m_lookupTable->SetValueRange(0.0,
                1.0); // from black to white
            component.m_lookupTable->SetAlphaRange(0, 1);
            component.m_lookupTable->SetSaturationRange(1.0,
                1.0); // no color saturation
            component.m_lookupTable->SetHueRange(0.056, 0.056);
            for (int i = 0; i < component.m_lookupTable->GetNumberOfColors();
                 i++)
            {
                double rgba[4] = { 0 };
                component.m_lookupTable->GetIndexedColor(i, rgba);
                if (rgba[0] <= m_thresholdOfBlackPixels[0]
                    && rgba[1] <= m_thresholdOfBlackPixels[1]
                    && rgba[2] <= m_thresholdOfBlackPixels[2])
                    component.m_lookupTable->SetTableValue(i, rgba[0], rgba[1],
                        rgba[2], 0);
                component.m_lookupTable->GetIndexedColor(i, rgba);
            }
            component.m_lookupTable->SetRampToLinear();
            component.m_lookupTable->ForceBuild();
        }
        for (auto& components : m_stereoComponents)
        {
            for (int j = 0; j < 3; j++)
            {
                components.m_lookupTable[j]->SetNumberOfTableValues(128);
                components.m_lookupTable[j]->SetValueRange(0.0,
                    1.0); // from black to white
                components.m_lookupTable[j]->SetAlphaRange(0, 1);
                components.m_lookupTable[j]->SetSaturationRange(1.0,
                    1.0); // no color saturation
                components.m_lookupTable[j]->SetHueRange(0.056, 0.056);
                for (int i = 0;
                     i < components.m_lookupTable[j]->GetNumberOfColors(); i++)
                {
                    double rgba[4] = { 0 };
                    components.m_lookupTable[j]->GetIndexedColor(i, rgba);
                    if (rgba[0] <= m_thresholdOfBlackPixels[0]
                        && rgba[1] <= m_thresholdOfBlackPixels[1]
                        && rgba[2] <= m_thresholdOfBlackPixels[2])
                        components.m_lookupTable[j]->SetTableValue(i, rgba[0],
                            rgba[1], rgba[2], 0);
                    components.m_lookupTable[j]->GetIndexedColor(i, rgba);
                }
                components.m_lookupTable[j]->SetRampToLinear();
                components.m_lookupTable[j]->ForceBuild();
            }
        }
    }
    else if (index == 2)
    {
        for (auto& component : m_sliceComponents)
        {
            component.m_lookupTable->SetNumberOfTableValues(128);
            component.m_lookupTable->SetValueRange(0.0,
                1.0); // from black to white
            component.m_lookupTable->SetAlphaRange(0, 1);
            component.m_lookupTable->SetSaturationRange(1.0,
                1.0); // no color saturation
            component.m_lookupTable->SetHueRange(0.59, 0.59);
            for (int i = 0; i < component.m_lookupTable->GetNumberOfColors();
                 i++)
            {
                double rgba[4] = { 0 };
                component.m_lookupTable->GetIndexedColor(i, rgba);
                if (rgba[0] <= m_thresholdOfBlackPixels[0]
                    && rgba[1] <= m_thresholdOfBlackPixels[1]
                    && rgba[2] <= m_thresholdOfBlackPixels[2])
                    component.m_lookupTable->SetTableValue(i, rgba[0], rgba[1],
                        rgba[2], 0);
                component.m_lookupTable->GetIndexedColor(i, rgba);
            }
            component.m_lookupTable->SetRampToLinear();
            component.m_lookupTable->ForceBuild();
        }
        for (auto& components : m_stereoComponents)
        {
            for (int j = 0; j < 3; j++)
            {
                components.m_lookupTable[j]->SetNumberOfTableValues(128);
                components.m_lookupTable[j]->SetValueRange(0.0,
                    1.0); // from black to white
                components.m_lookupTable[j]->SetAlphaRange(0, 1);
                components.m_lookupTable[j]->SetSaturationRange(1.0,
                    1.0); // no color saturation
                components.m_lookupTable[j]->SetHueRange(0.59, 0.59);
                for (int i = 0;
                     i < components.m_lookupTable[j]->GetNumberOfColors(); i++)
                {
                    double rgba[4] = { 0 };
                    components.m_lookupTable[j]->GetIndexedColor(i, rgba);
                    if (rgba[0] <= m_thresholdOfBlackPixels[0]
                        && rgba[1] <= m_thresholdOfBlackPixels[1]
                        && rgba[2] <= m_thresholdOfBlackPixels[2])
                        components.m_lookupTable[j]->SetTableValue(i, rgba[0],
                            rgba[1], rgba[2], 0);
                    components.m_lookupTable[j]->GetIndexedColor(i, rgba);
                }
                components.m_lookupTable[j]->SetRampToLinear();
                components.m_lookupTable[j]->ForceBuild();
            }
        }
    }
    else
    {
        for (auto& component : m_sliceComponents)
        {
            component.m_lookupTable->SetNumberOfTableValues(256);
            component.m_lookupTable->SetValueRange(0.0,
                1.0); // from black to white
            component.m_lookupTable->SetSaturationRange(0.0,
                0.0); // no color saturation
            component.m_lookupTable->SetHueRange(0.0, 0.0);
            component.m_lookupTable->SetRampToLinear();
            component.m_lookupTable->ForceBuild();
        }
        for (auto& components : m_stereoComponents)
        {
            for (int i = 0; i < 3; i++)
            {
                components.m_lookupTable[i]->SetNumberOfTableValues(256);
                components.m_lookupTable[i]->SetValueRange(0.0,
                    1.0); // from black to white
                components.m_lookupTable[i]->SetSaturationRange(0.0,
                    0.0); // no color saturation
                components.m_lookupTable[i]->SetHueRange(0.0, 0.0);
                components.m_lookupTable[i]->SetRampToLinear();
                components.m_lookupTable[i]->ForceBuild();
            }
        }
    }
    emit propertyUpdated();
}

void VolumeImage::switchColorMaps(bool next)
{
    int nextIndex = 0;
    for (int i = 0; i < m_schemesContainer.size(); ++i)
        if (m_currentColorScheme == m_schemesContainer.at(i))
        {
            if (next)
                nextIndex = (i + 1) % m_schemesContainer.size();
            else
                nextIndex = (i + m_schemesContainer.size() - 1)
                    % m_schemesContainer.size();
            break;
        }
    switchColorMaps(m_schemesContainer.at(nextIndex));
}

void VolumeImage::switchColorMaps(const vtkColorSeries::ColorSchemes& color)
{
    m_currentColorScheme = color;
    setColorMaps(color);
}

void VolumeImage::getVoxelDataRange(double range[2]) const
{
    range[0] = m_voxelDataRange[0];
    range[1] = m_voxelDataRange[1];
}

void VolumeImage::setContrastSingleStep()
{
    m_contrastSingleStep
        = (m_voxelDataRange[1] - m_voxelDataRange[0]) * m_contrastStepPercent;
}

void VolumeImage::storeContrastRange(
    std::shared_ptr<std::vector<std::pair<double, double>>> cRange)
{
    if (cRange != nullptr)
    {
        m_contrastRange = cRange;
        setContrastSingleStep();
    }
}

void VolumeImage::setContrastStep(bool increase)
{
    if (m_contrastStepPercent > 0 && m_contrastStepPercent < 1)
    {
        if (increase)
            m_contrastStepPercent += 0.01;
        else
            m_contrastStepPercent -= 0.01;
        setContrastSingleStep();
    }
}

void VolumeImage::getLookupTableRange(double range[2]) const
{
    for (auto& component : m_sliceComponents)
    {
        range[0] = component.m_lookupTable->GetRange()[0];
        range[1] = component.m_lookupTable->GetRange()[1];
    }
    for (auto& components : m_stereoComponents)
    {
        for (int i = 0; i < 3; i++)
        {
            range[0] = components.m_lookupTable[i]->GetRange()[0];
            range[1] = components.m_lookupTable[i]->GetRange()[1];
        }
    }
}

void VolumeImage::setLookupTableRange(double _min, double _max)
{
    if (_max < _min)
        return;
    for (auto& component : m_sliceComponents)
    {
        component.m_lookupTable->SetRange(_min, _max);
        component.m_lookupTable->Modified();
    }
    for (auto& components : m_stereoComponents)
    {
        for (int i = 0; i < 3; i++)
        {
            components.m_lookupTable[i]->SetRange(_min, _max);
            components.m_lookupTable[i]->Modified();
        }
    }
    emit propertyUpdated();
}

void VolumeImage::setWindow(bool increase)
{
    double step = getContrastSingleStep();
    double minValNmaxVal[2];
    getLookupTableRange(minValNmaxVal);
    if (increase)
        setLookupTableRange(minValNmaxVal[0] - step, minValNmaxVal[1] + step);
    else
        setLookupTableRange(minValNmaxVal[0] + step, minValNmaxVal[1] - step);
}

void VolumeImage::setLevel(bool increase)
{
    double step = getContrastSingleStep();
    double minValNmaxVal[2];
    getLookupTableRange(minValNmaxVal);
    if (increase)
        setLookupTableRange(minValNmaxVal[0] - step, minValNmaxVal[1] - step);
    else
        setLookupTableRange(minValNmaxVal[0] + step, minValNmaxVal[1] + step);
}

void VolumeImage::setAutoContrast(bool incresae)
{
    if (auto cRange = m_contrastRange.lock())
    {
        if (!incresae)
        {
            auto frontRange = cRange->front();
            cRange->erase(cRange->begin());
            cRange->push_back(frontRange);
        }
        else
        {
            auto backRange = cRange->back();
            cRange->pop_back();
            cRange->insert(cRange->begin(), backRange);
        }
        auto& range = cRange->front();
        setLookupTableRange(range.first, range.second);
    }
}

void VolumeImage::saveToNrrdFileWithItkApi(
    const itk::Image<short, 3>::Pointer& itkImagePointer, std::string output)
{
    int last = output.find_last_of('.');
    std::string path = output.substr(0, last + 1);
    std::string suffix = output.substr(last, output.size() - last);
    bool isFileExist = fileExists(QString::fromStdString(output));
    if (suffix == ".nrrd" && isFileExist)
    {
        return;
    }
    std::string target = path + "nrrd";

    using InputImageType = itk::Image<InputPixelType, 3>;
    // convert pixel type since it is saved for registration. When registration supports short pixel type it doesn't need to convert pixel type.
    using OutputPixelType = float;
    using OutputImageType = itk::Image<OutputPixelType, 3>;

    using CastFilterType
        = itk::CastImageFilter<InputImageType, OutputImageType>;
    CastFilterType::Pointer castFilter = CastFilterType::New();
    castFilter->SetInput(itkImagePointer);
    try
    {
        castFilter->Update();
    }
    catch (itk::ExceptionObject e)
    {
        std::cout << e << std::endl;
    }

    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileType(itk::ImageIOBase::Binary);

    typedef itk::MetaDataDictionary DictionaryType;
    DictionaryType& dictionary = io->GetMetaDataDictionary();
    using MetaDataStringType = itk::MetaDataObject<std::string>;
    MetaDataStringType::Pointer spaceMeta = MetaDataStringType::New();
    spaceMeta->SetMetaDataObjectValue("scanner-xyz");
    dictionary.Set("NRRD_space", spaceMeta);

    typedef itk::ImageFileWriter<OutputImageType> WriterType;
    WriterType::Pointer nrrdWriter = WriterType::New();
    nrrdWriter->UseInputMetaDataDictionaryOn();
    nrrdWriter->SetInput(castFilter->GetOutput());
    nrrdWriter->SetImageIO(io);
    nrrdWriter->SetMetaDataDictionary(dictionary);
    nrrdWriter->SetFileName(target);
    try
    {
        nrrdWriter->Update();
    }
    catch (itk::ExceptionObject& error)
    {
        qDebug() << "VolumeImage -- Failed to write nrrd file. File path: ("
                 << m_filePath << ") error: " << error.what();
    }
}

void VolumeImage::saveToNrrdFileByLocalApi(
    const itk::Image<short, 3>::Pointer& itkImagePointer, std::string output,
    std::string saveType)
{
    int last = output.find_last_of('.');
    std::string path = output.substr(0, last + 1);
    // check condition suffix == ".nrrd"
    std::string suffix = output.substr(last, output.size() - last);
    bool isFileExist = fileExists(QString::fromStdString(output));
    if (suffix == ".nrrd" && isFileExist)
    {
        return;
    }
    std::string target = path + "nrrd";
    short* itkImageDataPointer
        = itkImagePointer->GetPixelContainer()->GetBufferPointer();
    ////////////////get space directions values//////////
    using MatrixType = itk::Matrix<double, 3, 3>;
    MatrixType invertedMatrix = itkImagePointer->GetInverseDirection();
    MatrixType tempMatrix;
    tempMatrix(0, 0) = itkImagePointer->GetSpacing()[0];
    tempMatrix(0, 1) = 0.0f;
    tempMatrix(0, 2) = 0.0f;
    tempMatrix(1, 0) = 0.0f;
    tempMatrix(1, 1) = itkImagePointer->GetSpacing()[1];
    tempMatrix(1, 2) = 0.0f;
    tempMatrix(2, 0) = 0.0f;
    tempMatrix(2, 1) = 0.0f;
    tempMatrix(2, 2) = itkImagePointer->GetSpacing()[2];
    MatrixType finalSpaceDirection = invertedMatrix * tempMatrix;

    using InputImageType = itk::Image<short, 3>;
    std::string dataType = saveType;
    int itkImageDataSize = itkImagePointer->GetPixelContainer()->Size();
    float* floatTypeITKImageData = nullptr;
    short* shortTypeITKImageData = nullptr;
    if (saveType == "float")
    {
        ///////////cast image to float/////////
        using OutputImageType = itk::Image<float, 3>;
        using CastFilterType
            = itk::CastImageFilter<InputImageType, OutputImageType>;
        CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(itkImagePointer);
        try
        {
            castFilter->Update();
        }
        catch (itk::ExceptionObject& error)
        {
            qDebug() << "VolumeImage -- Failed to cast itk image: "
                     << error.what();
            return;
        }
        itk::Image<float, 3U>::Pointer floatTypeITKImage
            = castFilter->GetOutput();
        floatTypeITKImageData
            = floatTypeITKImage->GetPixelContainer()->GetBufferPointer();
    }
    else if (saveType == "uint8")
    {
        shortTypeITKImageData
            = itkImagePointer->GetPixelContainer()->GetBufferPointer();
    }
    // get dimension size of image
    InputImageType::RegionType region
        = itkImagePointer->GetLargestPossibleRegion();
    InputImageType::SizeType size = region.GetSize();

    ///////////////////////generate the nrrd file/////////////////////
    std::ofstream nrrdHeader;
    nrrdHeader.open(target, std::ofstream::out);
    nrrdHeader << "NRRD0004" << std::endl;
    nrrdHeader << "type: " << dataType << std::endl;
    nrrdHeader << "dimension: 3" << std::endl;
    nrrdHeader << "space: scanner-xyz" << std::endl;
    nrrdHeader << "sizes: " << size[0] << " " << size[1] << " " << size[2]
               << std::endl;

    // construct space directions with the format: (a,b,c) (d, e, f) (g, h, i)
    std::string spaceDirectionsString = "";
    for (int i = 0; i < 3; i++)
    {
        spaceDirectionsString += "(" + std::to_string(finalSpaceDirection[i][0])
            + "," + std::to_string(finalSpaceDirection[i][1]) + ","
            + std::to_string(finalSpaceDirection[i][2]) + ")";
        spaceDirectionsString += " ";
    }

    nrrdHeader << "space directions: " << spaceDirectionsString << std::endl;
    nrrdHeader << "kinds: domain domain domain" << std::endl;
    nrrdHeader << "endian: little" << std::endl;
    nrrdHeader << "encoding: raw" << std::endl;
    nrrdHeader << "space origin: (" << itkImagePointer->GetOrigin()[0] << ","
               << itkImagePointer->GetOrigin()[1] << ","
               << itkImagePointer->GetOrigin()[2] << ")" << std::endl
               << std::endl;
    nrrdHeader.close();

    // append writing the binary data of itk image
    std::ofstream binaryStream;
    binaryStream.open(target, std::ios::app | std::ios::binary);
    if (!binaryStream.good())
    {
        qDebug() << "VolumeImage -- Failed to open "
                 << QString::fromStdString(target);
        return;
    }
    ////////////////////////////////////////////////////////////////////////////
    /* the method reads the volume data by short type with ITK library, but writing the data into
    file, we need to write it byte by byte. So, conversion from short to char is nessesary. Or, the
    data will be written with many strides in image. short ->  char buffer -> write to file stream,
    if reading type has been changed, the ratio will be changed.
    */
    std::vector<unsigned char> rawImageData;
    rawImageData.resize(itkImageDataSize);
    std::copy(shortTypeITKImageData, shortTypeITKImageData + itkImageDataSize,
        rawImageData.data());

    if (saveType == "float")
    {
        binaryStream.write(reinterpret_cast<char*>(rawImageData.data()),
            sizeof(float) * itkImageDataSize);
    }
    else if (saveType == "uint8")
    {
        binaryStream.write(reinterpret_cast<char*>(rawImageData.data()),
            sizeof(unsigned char) * itkImageDataSize);
    }

    binaryStream.close();
}

bool VolumeImage::convertNonNrrdFileToNrrd(
    std::vector<std::string>& inputNonNrrdFiles, std::string saveLocalNrrdFile,
    std::string saveType)
{
    itk::NrrdImageIOFactory::RegisterOneFactory();
    itk::NiftiImageIOFactory::RegisterOneFactory();
    itk::MINCImageIOFactory::RegisterOneFactory();
    itk::MetaImageIOFactory::RegisterOneFactory();
    itk::GDCMImageIOFactory::RegisterOneFactory();

    itk::Image<short, 3>::Pointer itkImagePointer = nullptr;
    // check the size of inputNonNrrdFiles, only one file means this inputNonNrrdFile will be
    // mha,nrrd,.., more than one file will be dicom serial files
    if (inputNonNrrdFiles.size() == 1)
    {
        using InputImageType = itk::Image<InputPixelType, 3>;
        // 1. read the image from local file
        itk::ImageFileReader<InputImageType>::Pointer reader
            = itk::ImageFileReader<InputImageType>::New();
        reader->SetFileName(inputNonNrrdFiles.at(0));
        try
        {
            reader->Update();
        }
        catch (itk::ExceptionObject& e)
        {
            qDebug() << "Volume Image -- ITK image filter with error: "
                     << e.what();
            return false;
        }
        itkImagePointer = reader->GetOutput();
    }
    else
    {
        // reading DICOM subseries
        using ImageType = itk::Image<InputPixelType, 3>;
        using ReaderType
            = itk::ImageSeriesReader<itk::Image<InputPixelType, 3>>;
        using ImageIOType = itk::GDCMImageIO;
        using VtkFilter = itk::ImageToVTKImageFilter<ImageType>;

        auto reader = ReaderType::New();
        auto dicomIO = ImageIOType::New();
        auto vtkFilter = VtkFilter::New();

        reader->SetImageIO(dicomIO);
        reader->SetFileNames(inputNonNrrdFiles);
        reader->ForceOrthogonalDirectionOff();
        try
        {
            reader->Update();
        }
        catch (itk::ExceptionObject& err)
        {
            qDebug() << "Volume Image -- Failed to read dicom images: "
                     << err.what();
        }
        itkImagePointer = reader->GetOutput();
    }
    ////////////////get space directions values//////////
    using MatrixType = itk::Matrix<double, 3, 3>;
    MatrixType invertedMatrix = itkImagePointer->GetInverseDirection();
    MatrixType tempMatrix;
    tempMatrix(0, 0) = itkImagePointer->GetSpacing()[0];
    tempMatrix(0, 1) = 0.0f;
    tempMatrix(0, 2) = 0.0f;
    tempMatrix(1, 0) = 0.0f;
    tempMatrix(1, 1) = itkImagePointer->GetSpacing()[1];
    tempMatrix(1, 2) = 0.0f;
    tempMatrix(2, 0) = 0.0f;
    tempMatrix(2, 1) = 0.0f;
    tempMatrix(2, 2) = itkImagePointer->GetSpacing()[2];
    MatrixType finalSpaceDirection = invertedMatrix * tempMatrix;

    using InputImageType = itk::Image<short, 3>;
    std::string dataType = saveType;
    int itkImageDataSize = itkImagePointer->GetPixelContainer()->Size();
    float* floatTypeITKImageData = nullptr;
    short* shortTypeITKImageData = nullptr;

    // get dimension size of image
    InputImageType::RegionType region
        = itkImagePointer->GetLargestPossibleRegion();
    InputImageType::SizeType size = region.GetSize();

    ///////////////////////generate the nrrd file/////////////////////
    std::ofstream nrrdHeader;
    nrrdHeader.open(saveLocalNrrdFile, std::ofstream::out);
    nrrdHeader << "NRRD0004" << std::endl;
    nrrdHeader << "type: " << dataType << std::endl;
    nrrdHeader << "dimension: 3" << std::endl;
    nrrdHeader << "space: scanner-xyz" << std::endl;
    nrrdHeader << "sizes: " << size[0] << " " << size[1] << " " << size[2]
               << std::endl;

    // construct space directions with the format: (a,b,c) (d, e, f) (g, h, i)
    std::string spaceDirectionsString = "";
    for (int i = 0; i < 3; i++)
    {
        spaceDirectionsString += "(" + std::to_string(finalSpaceDirection[i][0])
            + "," + std::to_string(finalSpaceDirection[i][1]) + ","
            + std::to_string(finalSpaceDirection[i][2]) + ")";
        spaceDirectionsString += " ";
    }

    nrrdHeader << "space directions: " << spaceDirectionsString << std::endl;
    nrrdHeader << "kinds: domain domain domain" << std::endl;
    nrrdHeader << "endian: little" << std::endl;
    nrrdHeader << "encoding: raw" << std::endl;
    nrrdHeader << "space origin: (" << itkImagePointer->GetOrigin()[0] << ","
               << itkImagePointer->GetOrigin()[1] << ","
               << itkImagePointer->GetOrigin()[2] << ")" << std::endl
               << std::endl;
    nrrdHeader.close();

    // append writing the binary data of itk image
    std::ofstream binaryStream;
    binaryStream.open(saveLocalNrrdFile, std::ios::app | std::ios::binary);
    if (!binaryStream.good())
    {
        qDebug() << "VolumeImage -- Failed to open "
                 << QString::fromStdString(saveLocalNrrdFile);
        return false;
    }

    if (saveType == "float")
    {
        ///////////cast image to float/////////
        using OutputImageType = itk::Image<float, 3>;
        using CastFilterType
            = itk::CastImageFilter<InputImageType, OutputImageType>;
        CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(itkImagePointer);
        try
        {
            castFilter->Update();
        }
        catch (itk::ExceptionObject& error)
        {
            qDebug() << "VolumeImage -- Failed to cast itk image: "
                     << error.what();
            return false;
        }
        itk::Image<float, 3U>::Pointer floatTypeITKImage
            = castFilter->GetOutput();
        floatTypeITKImageData
            = floatTypeITKImage->GetPixelContainer()->GetBufferPointer();
        itkImageDataSize = floatTypeITKImage->GetPixelContainer()->Size();

        std::vector<unsigned char> rawImageData;
        rawImageData.resize(itkImageDataSize);
        std::copy(floatTypeITKImageData,
            floatTypeITKImageData + itkImageDataSize, rawImageData.data());
        binaryStream.write(reinterpret_cast<char*>(floatTypeITKImageData),
            sizeof(float) * itkImageDataSize);
    }
    else if (saveType == "uint8")
    {
        shortTypeITKImageData
            = itkImagePointer->GetPixelContainer()->GetBufferPointer();
        ////////////////////////////////////////////////////////////////////////////
        /* the method reads the volume data by short type with ITK library, but writing the data
        into file, we need to write it byte by byte. So, conversion from short to char is nessesary.
        Or, the data will be written with many strides in image. short ->  char buffer -> write to
        file stream, if reading type has been changed, the ratio will be changed.
        */
        std::vector<unsigned char> rawImageData;
        rawImageData.resize(itkImageDataSize);
        std::copy(shortTypeITKImageData,
            shortTypeITKImageData + itkImageDataSize, rawImageData.data());
        binaryStream.write(reinterpret_cast<char*>(rawImageData.data()),
            itkImageDataSize);
    }
    binaryStream.close();
    return true;
}

bool VolumeImage::convertNonNrrdFileToNrrdITK(
    std::vector<std::string>& inputNonNrrdFiles, std::string saveLocalNrrdFile,
    std::string saveType)
{
    itk::NrrdImageIOFactory::RegisterOneFactory();
    itk::NiftiImageIOFactory::RegisterOneFactory();
    itk::MINCImageIOFactory::RegisterOneFactory();
    itk::MetaImageIOFactory::RegisterOneFactory();
    itk::GDCMImageIOFactory::RegisterOneFactory();

    using ShortImageType = itk::Image<short, 3>;
    ShortImageType::Pointer itkImagePointer = nullptr;
    // check the size of inputNonNrrdFiles, only one file means this inputNonNrrdFile will be mha,nrrd,.., more than one file will be dicom serial files
    if (inputNonNrrdFiles.size() == 1)
    {
        using InputImageType = itk::Image<InputPixelType, 3>;
        // 1. read the image from local file
        itk::ImageFileReader<InputImageType>::Pointer reader
            = itk::ImageFileReader<InputImageType>::New();
        reader->SetFileName(inputNonNrrdFiles.at(0));
        try
        {
            reader->Update();
        }
        catch (itk::ExceptionObject& e)
        {
            qDebug() << "Volume Image -- ITK image filter with error: "
                     << e.what();
            return false;
        }
        itkImagePointer = reader->GetOutput();
    }
    else
    {
        // reading DICOM subseries
        using ImageType = itk::Image<InputPixelType, 3>;
        using ReaderType
            = itk::ImageSeriesReader<itk::Image<InputPixelType, 3>>;
        using ImageIOType = itk::GDCMImageIO;
        using VtkFilter = itk::ImageToVTKImageFilter<ImageType>;

        auto reader = ReaderType::New();
        auto dicomIO = ImageIOType::New();
        auto vtkFilter = VtkFilter::New();

        reader->SetImageIO(dicomIO);
        reader->SetFileNames(inputNonNrrdFiles);
        reader->ForceOrthogonalDirectionOff();
        try
        {
            reader->Update();
        }
        catch (itk::ExceptionObject& err)
        {
            qDebug() << "Volume Image -- Failed to read dicom images: "
                     << err.what();
        }
        itkImagePointer = reader->GetOutput();
    }
    short* itkImageDataPointer
        = itkImagePointer->GetPixelContainer()->GetBufferPointer();
    typedef itk::MetaDataDictionary DictionaryType;
    DictionaryType& dictionary = itkImagePointer->GetMetaDataDictionary();

    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileType(itk::ImageIOBase::Binary);

    /*using MetaDataStringType = itk::MetaDataObject<std::string>;
    MetaDataStringType::Pointer spaceMeta = MetaDataStringType::New();
    spaceMeta->SetMetaDataObjectValue("scanner-xyz");
    dictionary.Set("NRRD_space", spaceMeta);*/

    typedef itk::ImageFileWriter<ShortImageType> WriterType;
    WriterType::Pointer nrrdWriter = WriterType::New();
    nrrdWriter->UseInputMetaDataDictionaryOn();
    nrrdWriter->SetInput(itkImagePointer);
    nrrdWriter->SetImageIO(io);
    nrrdWriter->SetMetaDataDictionary(dictionary);

    nrrdWriter->SetFileName(
        QString::fromStdString(saveLocalNrrdFile).toLocal8Bit().toStdString());
    try
    {
        nrrdWriter->Update();
    }
    catch (itk::ExceptionObject& error)
    {
        qDebug() << "VolumeImage -- Failed to write nrrd file. error: "
                 << error.what();
        return false;
    }
    return true;
}

const bool VolumeImage::fileExists(const QString& filePath)
{
    QFileInfo pathInfo(filePath);
    return pathInfo.exists();
}
} // namespace utility
} // namespace interface
}; // namespace ultrast