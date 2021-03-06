/*
 * datasetmesh.cpp
 *
 * Created on: Jul 19, 2012
 * @author Ralph Schurade
 */
#include "datasetmesh.h"
#include "datasetscalar.h"

#include "../models.h"
#include "../vptr.h"

#include "../mesh/trianglemesh2.h"

#include "../../gui/gl/glfunctions.h"
#include "../../gui/gl/colormapfunctions.h"
#include "../../gui/gl/meshrenderer.h"

#include <QFile>
#include <QFileDialog>

DatasetMesh::DatasetMesh( TriangleMesh2* mesh, QDir fileName ) :
    Dataset( fileName, Fn::DatasetType::MESH_BINARY ),
    m_renderer( 0 )
{
    m_mesh.push_back( mesh );
    initProperties();
    finalizeProperties();
}

DatasetMesh::DatasetMesh( QDir fileName, Fn::DatasetType type ) :
    Dataset( fileName, type ),
    m_renderer( 0 )
{
    initProperties();
}

DatasetMesh::~DatasetMesh()
{
}

void DatasetMesh::initProperties()
{
    float min = 0.0;
    float max = 1.0;

    m_properties["maingl"].createRadioGroup( Fn::Property::D_COLORMODE, { "per mesh", "mri", "per vertex", "vertex data" }, 0, "general" );
    m_properties["maingl"].createBool( Fn::Property::D_INTERPOLATION, true, "general" );
    m_properties["maingl"].createInt( Fn::Property::D_COLORMAP, 1, "general" );
    m_properties["maingl"].createFloat( Fn::Property::D_SELECTED_MIN, min, min, max, "general"  );
    m_properties["maingl"].createFloat( Fn::Property::D_SELECTED_MAX, max, min, max, "general"  );
    m_properties["maingl"].createFloat( Fn::Property::D_LOWER_THRESHOLD, min + ( max - min ) / 1000., min, max, "general"  );
    m_properties["maingl"].createFloat( Fn::Property::D_UPPER_THRESHOLD, max, min, max, "general"  );

    m_properties["maingl"].createColor( Fn::Property::D_COLOR, QColor( 255, 255, 255 ), "general" );
    m_properties["maingl"].createFloat( Fn::Property::D_ALPHA, 1.f, 0.01f, 1.f, "general" );
    m_properties["maingl"].createButton( Fn::Property::D_COPY_COLORS, "general" );
    connect( m_properties["maingl"].getProperty( Fn::Property::D_COPY_COLORS ), SIGNAL( valueChanged( QVariant ) ), this, SLOT( slotCopyColors() ) );
    m_properties["maingl"].createButton( Fn::Property::D_COPY_VALUES, "general" );
    connect( m_properties["maingl"].getProperty( Fn::Property::D_COPY_VALUES ), SIGNAL( valueChanged( QVariant ) ), this, SLOT( slotCopyValues() ) );

    GLFunctions::createColormapBarProps( m_properties["maingl"] );

    m_properties["maingl"].createRadioGroup( Fn::Property::D_PAINTMODE, { "off", "paint", "paint values" }, 0, "paint" );
    m_properties["maingl"].createFloat( Fn::Property::D_PAINTSIZE, 20.f, 1.f, 1000.f, "paint" );
    m_properties["maingl"].createColor( Fn::Property::D_PAINTCOLOR, QColor( 255, 0, 0 ), "paint" );
    m_properties["maingl"].createFloat( Fn::Property::D_PAINTVALUE, 0.5f, -1.0f, 1.0f, "paint" );


    m_properties["maingl"].createFloat( Fn::Property::D_MIN, min );
    m_properties["maingl"].createFloat( Fn::Property::D_MAX, max );

    connect( m_properties["maingl"].getProperty( Fn::Property::D_SELECTED_MIN ), SIGNAL( valueChanged( QVariant ) ),
            m_properties["maingl"].getProperty( Fn::Property::D_LOWER_THRESHOLD ), SLOT( setMax( QVariant ) ) );
    connect( m_properties["maingl"].getProperty( Fn::Property::D_SELECTED_MAX ), SIGNAL( valueChanged( QVariant ) ),
            m_properties["maingl"].getProperty( Fn::Property::D_UPPER_THRESHOLD ), SLOT( setMin( QVariant ) ) );
    connect( m_properties["maingl"].getProperty( Fn::Property::D_PAINTMODE ), SIGNAL( valueChanged( QVariant ) ), this,
            SLOT( paintModeChanged( QVariant ) ) );

    m_properties["maingl"].createBool( Fn::Property::D_RENDER_WIREFRAME, false, "special" );

    m_properties["maingl"].createBool( Fn::Property::D_MESH_CUT_LOWER_X, false, "special" );
    m_properties["maingl"].createBool( Fn::Property::D_MESH_CUT_LOWER_Y, false, "special" );
    m_properties["maingl"].createBool( Fn::Property::D_MESH_CUT_LOWER_Z, false, "special" );
    m_properties["maingl"].createBool( Fn::Property::D_MESH_CUT_HIGHER_X, false, "special" );
    m_properties["maingl"].createBool( Fn::Property::D_MESH_CUT_HIGHER_Y, false, "special" );
    m_properties["maingl"].createBool( Fn::Property::D_MESH_CUT_HIGHER_Z, false, "special" );

    m_properties["maingl"].createFloat( Fn::Property::D_ADJUST_X, 0.0f, -500.0f, 500.0f, "special" );
    m_properties["maingl"].createFloat( Fn::Property::D_ADJUST_Y, 0.0f, -500.0f, 500.0f, "special" );
    m_properties["maingl"].createFloat( Fn::Property::D_ADJUST_Z, 0.0f, -500.0f, 500.0f, "special" );

    m_properties["maingl"].createFloat( Fn::Property::D_ROTATE_X, 0.0f, 0.0f, 360.0f, "special" );
    m_properties["maingl"].createFloat( Fn::Property::D_ROTATE_Y, 0.0f, 0.0f, 360.0f, "special" );
    m_properties["maingl"].createFloat( Fn::Property::D_ROTATE_Z, 0.0f, 0.0f, 360.0f, "special" );

    m_properties["maingl"].createFloat( Fn::Property::D_SCALE_X, 1.0f, 0.0f, 10.0f, "special" );
    m_properties["maingl"].createFloat( Fn::Property::D_SCALE_Y, 1.0f, 0.0f, 10.0f, "special" );
    m_properties["maingl"].createFloat( Fn::Property::D_SCALE_Z, 1.0f, 0.0f, 10.0f, "special" );

    m_properties["maingl"].createButton( Fn::Property::D_MESH_MAKE_PERMANENT, "special" );
    connect( m_properties["maingl"].getProperty( Fn::Property::D_MESH_MAKE_PERMANENT ), SIGNAL( valueChanged( QVariant ) ), this,
            SLOT( makePermanent() ) );

    m_properties["maingl"].createBool( Fn::Property::D_LIGHT_SWITCH, true, "light" );
    m_properties["maingl"].createFloat( Fn::Property::D_LIGHT_AMBIENT,   0.3f, 0.0f, 1.0f, "light" );
    m_properties["maingl"].createFloat( Fn::Property::D_LIGHT_DIFFUSE,   0.6f, 0.0f, 1.0f, "light" );
    m_properties["maingl"].createFloat( Fn::Property::D_LIGHT_SPECULAR,  0.5f, 0.0f, 1.0f, "light" );
    m_properties["maingl"].createFloat( Fn::Property::D_MATERIAL_AMBIENT,   0.5f, 0.0f, 10.0f, "light" );
    m_properties["maingl"].createFloat( Fn::Property::D_MATERIAL_DIFFUSE,   0.8f, 0.0f, 10.0f, "light" );
    m_properties["maingl"].createFloat( Fn::Property::D_MATERIAL_SPECULAR,  0.61f, 0.0f, 10.0f, "light" );
    m_properties["maingl"].createFloat( Fn::Property::D_MATERIAL_SHININESS, 1.0f, 0.0f, 200.0f, "light" );

    m_properties["maingl"].createInt( Fn::Property::D_MESH_NUM_VERTEX, 0 );
    m_properties["maingl"].createInt( Fn::Property::D_MESH_NUM_TRIANGLES, 0 );

    m_properties["maingl"].createInt( Fn::Property::D_GLYPHSET_PICKED_ID, 0 );

    m_properties["maingl"].createInt( Fn::Property::D_START_INDEX, 0 );
    m_properties["maingl"].createInt( Fn::Property::D_END_INDEX, 0 );

    if( m_mesh.size() > 0 )
    {
        m_properties["maingl"].set( Fn::Property::D_MESH_NUM_VERTEX, m_mesh[0]->numVerts() );
        m_properties["maingl"].set( Fn::Property::D_MESH_NUM_TRIANGLES, m_mesh[0]->numTris() );
        m_properties["maingl"].set( Fn::Property::D_START_INDEX, 0 );
        m_properties["maingl"].set( Fn::Property::D_END_INDEX, m_mesh[0]->numTris() );
    }

    m_properties["maingl"].createList( Fn::Property::D_USE_TRANSFORM, { "user defined", "qform", "sform", "qform inverted", "sform inverted" }, 0, "transform" );
    connect( m_properties["maingl"].getProperty( Fn::Property::D_USE_TRANSFORM ), SIGNAL( valueChanged( QVariant ) ), this,
                SLOT( transformChanged( QVariant ) ) );
    m_properties["maingl"].createMatrix( Fn::Property::D_TRANSFORM, m_transform, "transform" );
    m_properties["maingl"].createButton( Fn::Property::D_APPLY_TRANSFORM, "transform" );
    connect( m_properties["maingl"].getProperty( Fn::Property::D_APPLY_TRANSFORM ), SIGNAL( valueChanged( QVariant ) ), this,
                SLOT( applyTransform() ) );
    m_properties["maingl"].createBool( Fn::Property::D_INVERT_VERTEX_ORDER, false, "transform" );

    transformChanged( 0 );

}

void DatasetMesh::finalizeProperties()
{
    PropertyGroup props2( m_properties["maingl"] );
    m_properties.insert( "maingl2", props2 );
    m_properties["maingl2"].getProperty( Fn::Property::D_ACTIVE )->setPropertyTab( "general" );

    connect( m_properties["maingl2"].getProperty( Fn::Property::D_SELECTED_MIN ), SIGNAL( valueChanged( QVariant ) ),
            m_properties["maingl2"].getProperty( Fn::Property::D_LOWER_THRESHOLD ), SLOT( setMax( QVariant ) ) );
    connect( m_properties["maingl2"].getProperty( Fn::Property::D_SELECTED_MAX ), SIGNAL( valueChanged( QVariant ) ),
            m_properties["maingl2"].getProperty( Fn::Property::D_UPPER_THRESHOLD ), SLOT( setMin( QVariant ) ) );
    connect( m_properties["maingl2"].getProperty( Fn::Property::D_PAINTMODE ), SIGNAL( valueChanged( QVariant ) ), this,
            SLOT( paintModeChanged( QVariant ) ) );
}

void DatasetMesh::setProperties()
{
    m_properties["maingl"].createList( Fn::Property::D_SURFACE, m_displayList, 0, "general" );
    //m_properties["maingl2"].create( Fn::Property::D_SURFACE, m_displayList, 0, "general" );

    if( m_mesh.size() > 0 )
    {
        m_properties["maingl"].set( Fn::Property::D_START_INDEX, 0 );
        m_properties["maingl"].set( Fn::Property::D_END_INDEX, m_mesh[0]->numTris() );

    }
}


/*****************************************************************************************************************************************************
 *
 * adds TriangleMesh to the list of meshes only if those meshes have the exact same number of vertexes
 *
 ****************************************************************************************************************************************************/
void DatasetMesh::addMesh( TriangleMesh2* tm, QString displayString )
{
    if ( m_mesh.size() > 0 )
    {
        unsigned int numVerts = m_mesh[0]->numVerts();
        if ( numVerts != tm->numVerts() )
        {
            return;
        }
    }
    m_mesh.push_back( tm );
    m_displayList.push_back( displayString );
    m_properties["maingl"].set( Fn::Property::D_START_INDEX, 0 );
    m_properties["maingl"].set( Fn::Property::D_END_INDEX, m_mesh[0]->numTris() );
}

int DatasetMesh::getNumberOfMeshes()
{
    return m_mesh.size();
}

TriangleMesh2* DatasetMesh::getMesh()
{
    if ( m_mesh.size() > 0 )
    {
        return m_mesh[0];
    }
    return 0;
}

TriangleMesh2* DatasetMesh::getMesh( int id )
{
    if ( (int)m_mesh.size() > id )
    {
        return m_mesh[id];
    }
    return 0;
}

TriangleMesh2* DatasetMesh::getMesh( QString target )
{
    int n = properties( target ).get( Fn::Property::D_SURFACE ).toInt();
    return m_mesh[n];
}


void DatasetMesh::draw( QMatrix4x4 pMatrix, QMatrix4x4 mvMatrix, int width, int height, int renderMode, QString target )
{
    if ( !properties( target ).get( Fn::Property::D_ACTIVE ).toBool() )
    {
        return;
    }

    if ( m_resetRenderer )
    {
        delete m_renderer;
        m_renderer = 0;
        m_resetRenderer = false;
    }

    if ( m_renderer == 0 )
    {
        m_renderer = new MeshRenderer( getMesh(target) );
        m_renderer->init();
    }
    m_renderer->setMesh( getMesh(target) );
    m_renderer->draw( pMatrix, mvMatrix, width, height, renderMode, properties( target ) );

    GLFunctions::drawColormapBar( properties( target ), width, height, renderMode );
}

bool DatasetMesh::mousePick( int pickId, QVector3D pos, Qt::KeyboardModifiers modifiers, QString target )
{
    int paintMode = m_properties[target].get( Fn::Property::D_PAINTMODE ).toInt();
    if ( pickId == 0 || paintMode == 0 || !( modifiers & Qt::ControlModifier ) )
    {
        return false;
    }

    QColor color;
    if ( ( modifiers & Qt::ControlModifier ) && !( modifiers & Qt::ShiftModifier ) )
    {
        color = m_properties["maingl"].get( Fn::Property::D_PAINTCOLOR ).value<QColor>();
    }
    else if ( ( modifiers & Qt::ControlModifier ) && ( modifiers & Qt::ShiftModifier ) )
    {
        color = m_properties["maingl"].get( Fn::Property::D_COLOR ).value<QColor>();
    }
    else
    {
        return false;
    }

    std::vector<unsigned int> picked = getMesh( target )->pick( pos, m_properties["maingl"].get( Fn::Property::D_PAINTSIZE ).toFloat() );

    if ( picked.size() > 0 )
    {
        m_renderer->beginUpdateColor();
        for ( unsigned int i = 0; i < picked.size(); ++i )
        {
            m_renderer->updateColor( picked[i], color.redF(), color.greenF(), color.blueF(), 1.0 );
            for ( unsigned int m = 0; m < m_mesh.size(); ++m )
            {
                if ( paintMode == 1 ) //paint color
                {
                    m_mesh[m]->setVertexColor( picked[i], color );
                }
                else if ( paintMode == 2 ) //paint values
                {
                    //float value = m_mesh[0]->getVertexData( picked[i] ) + m_properties[target]->get( Fn::Property::D_PAINTVALUE ).toFloat();
                    float value = m_properties[target].get( Fn::Property::D_PAINTVALUE ).toFloat();
                    if ( value < 0.0 )
                    {
                        value = 0.0;
                    }
                    if ( value > 1.0 )
                    {
                        value = 1.0;
                    }
                    if ( ( modifiers & Qt::ShiftModifier ) )
                    {
                        value = 0.0;
                    }
                    m_mesh[m]->setVertexData( picked[i], value );
                }
            }
        }
        m_renderer->endUpdateColor();
        return true;
    }

    return false;
}

void DatasetMesh::paintModeChanged( QVariant mode )
{
    if ( mode.toInt() == 1 )
    {
        m_properties["maingl"].set( Fn::Property::D_COLORMODE, 2 );
    }
    if ( mode.toInt() == 2 )
    {
        m_properties["maingl"].set( Fn::Property::D_COLORMODE, 3 );
    }
}

void DatasetMesh::makePermanent()
{
    QMatrix4x4 mMatrix;
    mMatrix.setToIdentity();

    if( m_properties["maingl"].contains( Fn::Property::D_ROTATE_X ) )
    {
        mMatrix.rotate( -m_properties["maingl"].get( Fn::Property::D_ROTATE_X ).toFloat(), 1.0, 0.0, 0.0 );
        mMatrix.rotate( -m_properties["maingl"].get( Fn::Property::D_ROTATE_Y ).toFloat(), 0.0, 1.0, 0.0 );
        mMatrix.rotate( -m_properties["maingl"].get( Fn::Property::D_ROTATE_Z ).toFloat(), 0.0, 0.0, 1.0 );
        mMatrix.scale( m_properties["maingl"].get( Fn::Property::D_SCALE_X ).toFloat(),
                m_properties["maingl"].get( Fn::Property::D_SCALE_Y ).toFloat(),
                m_properties["maingl"].get( Fn::Property::D_SCALE_Z ).toFloat() );
    }

    int adjustX = m_properties["maingl"].get( Fn::Property::D_ADJUST_X ).toFloat();
    int adjustY = m_properties["maingl"].get( Fn::Property::D_ADJUST_Y ).toFloat();
    int adjustZ = m_properties["maingl"].get( Fn::Property::D_ADJUST_Z ).toFloat();

    int numVerts = m_mesh[0]->numVerts();

    for( int i = 0; i < numVerts; ++i )
    {
        QVector3D vert = m_mesh[0]->getVertex( i );

        vert = vert * mMatrix;
        vert.setX( vert.x() + adjustX );
        vert.setY( vert.y() + adjustY );
        vert.setZ( vert.z() + adjustZ );

        m_mesh[0]->setVertex( i, vert );
    }

    m_mesh[0]->finalize();

    m_resetRenderer = true;

    m_properties["maingl"].set( Fn::Property::D_ROTATE_X, 0 );
    m_properties["maingl"].set( Fn::Property::D_ROTATE_Y, 0 );
    m_properties["maingl"].set( Fn::Property::D_ROTATE_Z, 0 );

    m_properties["maingl"].set( Fn::Property::D_ADJUST_X, 0 );
    m_properties["maingl"].set( Fn::Property::D_ADJUST_Y, 0 );
    m_properties["maingl"].set( Fn::Property::D_ADJUST_Z, 0 );

    m_properties["maingl"].set( Fn::Property::D_SCALE_X, 1 );
    m_properties["maingl"].set( Fn::Property::D_SCALE_Y, 1 );
    m_properties["maingl"].set( Fn::Property::D_SCALE_Z, 1 );

    Models::d()->submit();
}

QString DatasetMesh::getSaveFilter()
{
    return QString( "Mesh binary (*.vtk);; Mesh ascii (*.asc);; Mesh 1D data (*.1D);; Mesh rgb data (*.rgb);; Mesh obj (*.obj);; Mesh VRML (*.wrl);; Mesh JSON (*.json);; all files (*.*)" );
}

QString DatasetMesh::getDefaultSuffix()
{
    return QString( "vtk" );
}

void DatasetMesh::transformChanged( QVariant value )
{
    QMatrix4x4 qForm;
    QMatrix4x4 sForm;

    QList<Dataset*>dsl = Models::getDatasets( Fn::DatasetType::NIFTI_ANY );

    if ( dsl.size() > 0 )
    {
        qForm = dsl.first()->properties().get( Fn::Property::D_Q_FORM ).value<QMatrix4x4>();
        sForm = dsl.first()->properties().get( Fn::Property::D_S_FORM ).value<QMatrix4x4>();
    }
    m_properties["maingl"].getWidget( Fn::Property::D_TRANSFORM )->setEnabled( false );

    switch ( value.toInt() )
    {
        case 0:
            m_transform.setToIdentity();
            m_properties["maingl"].getWidget( Fn::Property::D_TRANSFORM )->setEnabled( true );
            break;
        case 1:
            m_transform = qForm;
            break;
        case 2:
            m_transform = sForm;
            break;
        case 3:
            m_transform = qForm.inverted();
            break;
        case 4:
            m_transform = sForm.inverted();
            break;
        default:
            m_transform.setToIdentity();
            break;
    }

    m_properties["maingl"].set( Fn::Property::D_TRANSFORM, m_transform );
    Models::d()->submit();
}

void DatasetMesh::applyTransform()
{
    m_transform = m_properties["maingl"].get( Fn::Property::D_TRANSFORM ).value<QMatrix4x4>();

    int n = properties( "maingl" ).get( Fn::Property::D_SURFACE ).toInt();
    TriangleMesh2* mesh = m_mesh[n];

    m_transform = m_properties["maingl"].get( Fn::Property::D_TRANSFORM ).value<QMatrix4x4>();

    for ( unsigned int i = 0; i < mesh->numVerts(); ++i )
    {
        QVector3D vert = mesh->getVertex( i );
        vert = m_transform * vert;
        mesh->setVertex( i, vert );
    }

    mesh->finalize();

    m_resetRenderer = true;
    transformChanged( 0 );
    Models::d()->submit();
}

void DatasetMesh::slotCopyColors()
{
    int n = properties( "maingl" ).get( Fn::Property::D_SURFACE ).toInt();
    TriangleMesh2* mesh = m_mesh[n];

    if( m_properties["maingl"].get( Fn::Property::D_COLORMODE ).toInt() == 3 )
    {
        QColor color;

        float selectedMin = properties( "maingl" ).get( Fn::Property::D_SELECTED_MIN ).toFloat();
        float selectedMax = properties( "maingl" ).get( Fn::Property::D_SELECTED_MAX ).toFloat();

        m_renderer->beginUpdateColor();
        for ( unsigned int i = 0; i < mesh->numVerts(); ++i )
        {
            ColormapBase cmap = ColormapFunctions::getColormap( properties( "maingl" ).get( Fn::Property::D_COLORMAP ).toInt() );

            float value = ( mesh->getVertexData( i ) - selectedMin ) / ( selectedMax - selectedMin );
            color = cmap.getColor( qMax( 0.0f, qMin( 1.0f, value ) ) );

            mesh->setVertexColor( i, color );
        }
        m_renderer->endUpdateColor();
    }
    else
    {
        QList<QVariant>dsl =  Models::d()->data( Models::d()->index( 0, (int)Fn::Property::D_DATASET_LIST ), Qt::DisplayRole ).toList();

        QList<DatasetScalar*> texList;
        for ( int k = 0; k < dsl.size(); ++k )
        {
            Dataset* ds = VPtr<Dataset>::asPtr( dsl[k] );
            if ( ds->properties().get( Fn::Property::D_ACTIVE ).toBool() && ds->properties().get( Fn::Property::D_HAS_TEXTURE ).toBool() )
            {
                texList.push_back( VPtr<DatasetScalar>::asPtr( dsl[k] ) );
                if ( texList.size() == 5 )
                {
                    break;
                }
            }
        }

        if ( texList.empty() )
        {
            return;
        }

        //TriangleMesh2* mesh = getMesh();
        m_renderer->beginUpdateColor();
        for ( unsigned int i = 0; i < mesh->numVerts(); ++i )
        {
            QColor c = texList[0]->getColorAtPos( mesh->getVertex( i ) );
            for ( int k = 1; k < texList.size(); ++k )
            {
                QColor c2 = texList[k]->getColorAtPos( mesh->getVertex( i ) );
                c.setRedF( ( 1.0 - c2.alphaF() ) * c.redF() + c2.alphaF() * c2.redF() );
                c.setGreenF( ( 1.0 - c2.alphaF() ) * c.greenF() + c2.alphaF() * c2.greenF() );
                c.setBlueF( ( 1.0 - c2.alphaF() ) * c.blueF() + c2.alphaF() * c2.blueF() );
            }
            if ( c.redF() + c.greenF() + c.blueF() > 0 )
            {
                mesh->setVertexColor( i, c );
            }
            else
            {
                QColor col = m_properties["maingl"].get( Fn::Property::D_COLOR ).value<QColor>();
                mesh->setVertexColor( i, col );
            }
        }
        m_renderer->endUpdateColor();
    }
}

void DatasetMesh::slotCopyValues()
{
    int n = properties( "maingl" ).get( Fn::Property::D_SURFACE ).toInt();
    TriangleMesh2* mesh = m_mesh[n];

    QList<QVariant>dsl =  Models::d()->data( Models::d()->index( 0, (int)Fn::Property::D_DATASET_LIST ), Qt::DisplayRole ).toList();

    QList<Dataset*> texList = Models::getDatasets( Fn::DatasetType::NIFTI_SCALAR );

    if ( texList.empty() )
    {
        return;
    }

    float min = std::numeric_limits<float>::max();
    float max = -std::numeric_limits<float>::max();

    DatasetScalar* ds = dynamic_cast<DatasetScalar*>( texList[0] );
    //TriangleMesh2* mesh = getMesh();
    m_renderer->beginUpdateColor();
    for ( unsigned int i = 0; i < mesh->numVerts(); ++i )
    {
        float value = ds->getValueAtPos( mesh->getVertex( i ) );
        min = qMin( value, min );
        max = qMax( value, max );
        mesh->setVertexData( i, value );
    }
    m_renderer->endUpdateColor();
    m_properties["maingl"].getProperty( Fn::Property::D_MIN )->setValue( min );
    m_properties["maingl"].getProperty( Fn::Property::D_MAX )->setValue( max );
    m_properties["maingl"].getProperty( Fn::Property::D_SELECTED_MIN )->setValue( min );
    m_properties["maingl"].getProperty( Fn::Property::D_SELECTED_MAX )->setValue( max );
    m_properties["maingl"].getProperty( Fn::Property::D_LOWER_THRESHOLD )->setValue( min );
    m_properties["maingl"].getProperty( Fn::Property::D_UPPER_THRESHOLD )->setValue( max );
    m_properties["maingl"].set( Fn::Property::D_COLORMODE, 3 );
}
