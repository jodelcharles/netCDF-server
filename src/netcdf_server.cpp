#include "netcdf_server.h"

// TO-DO: replace vector copying with move semantics - DONE
// TO-DO: Clean up, thread safety, etc - DONE (mostly)
// TO-DO: any more considerations for race conditions
// TO-DO: break up handleGetImage()
// TO-DO: actual logging with CROW_LOGGING or some other method - DEFERRED

thread_local uint NetCDFServer :: timeIndex_;
thread_local uint NetCDFServer :: zIndex_;
thread_local std :: vector<double> NetCDFServer :: concentrationData_;

NetCDFServer :: NetCDFServer( const std :: string& fileName ) : fileName_( fileName ), 
                                                                dataFile_( fileName, NcFile :: read ),
                                                                responseCode_( 200 )
{
    timeIndex_  =   0;
    zIndex_     =   0;

    // force matplot++ to not open gnuplot #eyeroll
    setenv( "QT_QPA_PLATFORM", "offscreen", 1 );
}

void NetCDFServer :: run( uint port ) 
{
    CROW_ROUTE( app_, "/get-info" )
    ( [ this ]( const Request& request ) 
    {
        auto query      { request.raw_url };

        if( query.find( '?' ) != std :: string :: npos)
        {
            JSONValue result;
            result[ "error" ] = Errors :: REMOVE_PARMS;
            return JSONResponse( result, APPLICATION_JSON );
        }

        return handleGetInfo();
    } );

    CROW_ROUTE( app_, "/get-data" )
    ( [ this ]( const Request& request )
    {
        return handleGetData( request );
    } );

    CROW_ROUTE( app_, "/get-image" )
    ( [ this ]( const Request& request ) 
    {
        return handleGetImage( request );
    } );

    // start on localhost port 18080
    //app_.bindaddr( "127.0.0.1" ).port( port ).multithreaded().run();  // for local build
    app_.bindaddr( "0.0.0.0" ).port( port ).multithreaded().run();      
}


/*++++++++++++++++*
|  handleGetInfo  |
*+++++++++++++++++/

/*!
    Function for get-info: Returns the NetCDF detailed information
    (equivalent of `ncdump -h` with dimensions, variables, and global attributes).
*/
Response NetCDFServer :: handleGetInfo()  
{
    // json wrapper for server response
    JSONValue result;
    
    // response object
    Response response;

    try 
    { 
        /*---------------*
        | get dimensions |
        *---------------*/
        extractDimensions( result );

        /*--------------*
        | get variables |
        *--------------*/
        extractVariables( result );

        /*----------------------*
        | get global attributes |
        *----------------------*/
        extractGlobalAttributes( result );
    } 
    catch ( const std :: exception& e )  
    {
        result[ kError ] = Errors :: FAIL_R_NCDF + e.what();
        responseCode_ = 500;
    }

    response = JSONResponse( result, APPLICATION_JSON );

    return response;
}

// get dimensions from dataFile
void NetCDFServer :: extractDimensions( JSONValue& result )
{
    JSONMap dimensions;
    for( const auto& dim : dataFile_.getDims() )  
    {
        dimensions[ dim.first ] = dim.second.getSize();
    }
    result[ "dimensions" ] = std :: move( dimensions );
}

// get variables from dataFile
void NetCDFServer :: extractVariables( JSONValue& result )
{
    JSONValue::object variables;
    for( const auto& var : dataFile_.getVars() )  
    {
        JSONValue::object varInfo;

        // store variable type
        varInfo[ "type" ] = var.second.getType().getName();

        // store variable dimensions
        JSONList dimensionNames;
        for( const auto& dim : var.second.getDims() )  
        {
            dimensionNames.push_back( dim.getName() );
        }
        varInfo[ "dimensions" ] = std :: move( dimensionNames );

        // get attributes
        for( const auto& attr : var.second.getAtts() )  
        {
            std :: string value;
            switch ( attr.second.getType().getId() )  
            {
                case NcType :: nc_CHAR: 
                {
                    char charVal[ 256 ] = { 0 };
                    attr.second.getValues( charVal );
                    value = std :: string( charVal );
                    break;
                }
                case NcType :: nc_INT: 
                {
                    int intVal;
                    attr.second.getValues( &intVal );
                    value = std :: to_string( intVal );
                    break;
                }
                case NcType :: nc_FLOAT: 
                {
                    float floatVal;
                    attr.second.getValues( &floatVal );
                    value = std :: to_string( floatVal );
                    break;
                }
                case NcType :: nc_DOUBLE: 
                {
                    double doubleVal;
                    attr.second.getValues( &doubleVal );
                    value = std :: to_string( doubleVal );
                    break;
                }
                default:
                    value = Errors :: UNKNOWN_TYPE;
                    break;
            }
            varInfo[ attr.first ] = value; 
        }

        variables[ var.first ] = std :: move( varInfo );
    }
    result[ "variables" ] = std :: move( variables );
}

//get global attributes from dataFile
void NetCDFServer :: extractGlobalAttributes( JSONValue& result )
{
    JSONMap globalAttributes;
    for( const auto& attr : dataFile_.getAtts() )  
    {
        std :: string value;
        switch ( attr.second.getType().getId() )  
        {
            case NcType :: nc_CHAR: 
            {
                char charVal[ 256 ] = { 0 };
                attr.second.getValues( charVal );
                globalAttributes[ attr.first ] = std :: string( charVal );
                break;
            }
            case NcType :: nc_INT: 
            {
                int intVal;
                attr.second.getValues( &intVal );
                globalAttributes[ attr.first ] = intVal;
                break;
            }
            case NcType :: nc_FLOAT: 
            {
                float floatVal;
                attr.second.getValues( &floatVal );
                globalAttributes[ attr.first ] = floatVal;
                break;
            }
            case NcType :: nc_DOUBLE: 
            {
                double doubleVal;
                attr.second.getValues( &doubleVal );
                globalAttributes[ attr.first ] = doubleVal;
                break;
            }
            default:
                globalAttributes[ attr.first ] = Errors :: UNKNOWN_TYPE;
                break;
        }
    }
    result[ "global_attributes" ] = std :: move( globalAttributes );
}


/*++++++++++++++++*
|  handleGetData  |
*+++++++++++++++++/

/*!
    function for b. /get-data, params to include time index and z ( height )  index, 
    returns json response that includes x, y, and concentration data.
*/
Response NetCDFServer :: handleGetData( const Request& request )
{
    JSONValue   result;
    Response    response;

    if( !validateRequestParameters( request, 
                                    result, 
                                    timeIndex_, 
                                    zIndex_ ) )
        return JSONResponse( result, APPLICATION_JSON );

    result      =   extractNetCDFSlice( timeIndex_, zIndex_ );
    response    =   JSONResponse( result, APPLICATION_JSON );
    
    return response;
}


/*+++++++++++++++++*
|  handleGetImage  |
*++++++++++++++++++/ 

/*!
    function for c. get-image -  params to include time index and z index, 
    returns png visualization of concentration.
*/
Response NetCDFServer :: handleGetImage( const Request& request )
{
    JSONValue   result;
    Response    response;

    if( !validateRequestParameters( request,
                                    result,
                                    timeIndex_,
                                    zIndex_ ) ) 
        return JSONResponse( result, APPLICATION_JSON );

    // extract into conentrationData_
    result = extractNetCDFSlice( timeIndex_, zIndex_ );

    if ( result.count( kError ) > 0 ) 
    {
        responseCode_ = 500;
        return JSONResponse( result, APPLICATION_JSON );
    }

    // get into 2D array
    auto xSize = result[ kX ].size();
    auto ySize = result[ kY ].size();

    std :: vector<std :: vector<double>> grid( ySize, std :: vector<double>( xSize ) );

    for( size_t i = 0; i < ySize; i++ ) 
    {
        for( size_t j = 0; j < xSize; j++ ) 
        {
            grid[ i ][ j ] = concentrationData_[ i * xSize + j ];
        }
    }

    // create unique image filename based on params and timestamp
    std :: string uniqueImagePath = "assets/output_" + 
                                    std :: to_string( timeIndex_ ) + 
                                    "_" +
                                    std :: to_string( zIndex_ ) + 
                                    "_" +
                                    std :: to_string( std :: chrono :: system_clock :: now().time_since_epoch().count() ) +
                                    ".png";

    // gen image
    result = generateVisual( grid, uniqueImagePath );

    if( result.count( kError ) > 0 )
    {
        responseCode_ = 500;
        return JSONResponse( result, APPLICATION_JSON );
    }

    // give some time for generateVisual to complete, check every 100 mills, time out after 2 seconds
    if ( !waitForFile( uniqueImagePath, 2000, 100 ) ) 
    {  
        responseCode_ = 500;
        result[ kError ] = Errors :: PNG_TIMEOUT;
        return JSONResponse( result, APPLICATION_JSON );
    }

    std :: ifstream file;

    // read image and return response
    try
    {
        file.open( uniqueImagePath, std :: ios :: binary );
        if ( !file ) 
        {    
            throw std :: runtime_error( Errors :: FAIL_O_IMG );
        }
    }
    catch( const std :: exception& e )
    {
        responseCode_ = 500;
        result[ kError ] = Errors :: FAIL_O_IMG + e.what();
        return JSONResponse( result, APPLICATION_JSON );
    }
    
    std :: ostringstream imageBuffer;
    imageBuffer << file.rdbuf();

    response.code = 200;
    response.body = imageBuffer.str();

    response.set_header( "Content-Type", IMAGE_PNG );
    response.set_header( "Cache-Control", NO_CACHE_NO_STORE );

    // remove png
    std :: filesystem :: remove( uniqueImagePath );
    
    return response;
}

// extract one x, y slice given z and time
JSONValue NetCDFServer :: extractNetCDFSlice( int timeIndex, int zIndex )  
{
    JSONValue result;

    try 
    {
        // retrieve x and y variables
        auto x = dataFile_.getVar( kX );
        auto y = dataFile_.getVar( kY );

        std :: vector<double> xData( x.getDim( 0 ).getSize() );
        std :: vector<double> yData( y.getDim( 0 ).getSize() );

        // use getVar( double * dataValues ) to fill the vectors from the x and y variables
        x.getVar( xData.data() );
        y.getVar( yData.data() );

        // retrieve the concentration variable
        auto concentration = dataFile_.getVar( kConcentration );
        
        // initialize vector to receive x*y concentration doubles
        concentrationData_.resize( yData.size() * xData.size() );

        // use getVar( vector start, vector count, double * dataValues ) to fill the concentration vector of doubles
        concentration.getVar
        ( 
            {
                static_cast<size_t>( timeIndex ), static_cast<size_t>( zIndex ), 0, 0
            },
            {
                1, 1, yData.size() , xData.size() 
            },
            concentrationData_.data() 
         );

        // store results in JSON
        JSONList xList( xData.begin() , xData.end() );
        JSONList yList( yData.begin() , yData.end() );
        JSONList concentrationList;

        for( size_t i = 0; i < yData.size(); i++ )  
        {
            JSONList row;

            for( size_t j = 0; j < xData.size(); j++ )  
            {
                row.push_back( concentrationData_[ i * xData.size() + j ] );
            }

            concentrationList.push_back( std :: move( row ) );
        }

        result[ kX ] = std :: move( xList );
        result[ kY ] = std :: move( yList );
        result[ kConcentration ] = std :: move( concentrationList );
    } 
    catch ( const std :: exception& e )  
    {
        result[ kError ] = Errors :: EXTRACT_NCDF + e.what();
    }
    return result;
}

// this is to give time for the png to be created, check every pollInvervalMs mills
bool NetCDFServer :: waitForFile( const std :: string& path, 
                                  int timeoutMs, 
                                  int pollIntervalMs )
{
    using namespace std::chrono;

    auto start = steady_clock :: now();

    while ( duration_cast<milliseconds>( steady_clock :: now() - start ).count() < timeoutMs ) 
    {
        if ( std :: filesystem :: exists( path ))  
        {
            return true;
        }
        std :: this_thread :: sleep_for( milliseconds( pollIntervalMs ) );
    }
    return false;
}

// validate params and populate variables
bool NetCDFServer :: validateRequestParameters( const Request& request, 
                                                JSONValue& result,
                                                uint& timeIndex,
                                                uint& zIndex ) 
{
    auto query      { request.url_params };

    // extract params and return error if time and height are missing
    if ( !query.get( "time" )  || !query.get( "z" ) )  
    {
        result[ kError ] = Errors :: MISSING_PARMS;
        return false;
    }

    try
    {
        timeIndex   = std :: stoi( query.get( "time" ) );
        zIndex      = std :: stoi( query.get( "z" ) );
    }
    catch( const std :: exception& e )
    {
        responseCode_ = 500;
        result[ kError ] = Errors :: FAIL_STOI + e.what();
        return false;
    }

    // reject request if any other parameters are included
    for( const auto& key : query.keys() )  
    {
        if ( std :: string( key ) != "time" && std :: string( key ) != "z" )  
        {
            result[ kError ] = Errors :: INVALID_PARM + std :: string( key )  + ".";
            return false;
        }
    }

    // make sure we're within the bounds of time and depth dimensions
    size_t timeSize     =   dataFile_.getDim( "time" ).getSize();
    size_t zSize        =   dataFile_.getDim( "z" ).getSize();

    if ( timeIndex < 0 || timeIndex >= timeSize )  
    {
        result[ kError ] = dataFile_.getDim( "time" ).getName() + Errors :: INDEX_OOR + std :: to_string( timeSize - 1 )  + ".";
        return false;
    }
    if ( zIndex < 0 || zIndex >= zSize )  
    {
        result[ kError ] = dataFile_.getDim( "z" ).getName() + Errors :: INDEX_OOR + std :: to_string( zSize - 1 )  + ".";
        return false;
    }

    return true;
}

// response object
Response NetCDFServer :: JSONResponse( JSONValue json, std :: string contentType ) 
{
    // set content-type header to application/json
    Response response;
    response.set_header( "Content-Type", contentType );
    response.set_header( "Cache-Control", NO_CACHE_NO_STORE );
    
    response.code = responseCode_;
    
    // add indentation for readability
    response.body = json.dump( 3 ); 
    return response;
}

// use matplot++ to generate a png 
JSONValue NetCDFServer :: generateVisual( const std :: vector<std :: vector<double>>& grid, const std :: string& outputPath ) 
{
    JSONValue result;

    size_t rows = grid.size();
    size_t cols = grid[ 0 ].size();

    // convert 2D grid for matplot
    std :: vector<std :: vector<double>> z( rows, std :: vector<double>( cols ) );

    for( size_t i = 0; i < rows; i++ ) 
    {
        for( size_t j = 0; j < cols; j++ ) 
        {
            z[ i ][ j ] = grid[ i ][ j ];
        }
    }

    // create heatmap 
    imagesc( z );  
    colorbar();
    title( "Concentration Heatmap" );

    if ( grid.empty() || grid[ 0 ].empty() ) 
    {
        std :: cerr << Errors :: GRID_EMPTY << std :: endl;
        result[ kError ] = Errors :: GRID_EMPTY;
        return result;
    }

    // Save the heatmap image
    try
    {
        save( outputPath );
    }
    catch( const std :: exception& e )
    {
        std :: cerr << Errors :: FAIL_S_IMG << e.what() << std :: endl;
        result[ kError ] = Errors :: FAIL_S_IMG + e.what();
        return result;
    }

    return result;
}