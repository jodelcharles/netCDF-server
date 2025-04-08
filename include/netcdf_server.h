#ifndef NETCDF_SERVER_H
#define NETCDF_SERVER_H

#include "crow.h"
#include "netcdf/netcdf.h"
#include "netcdf/ncFile.h"
#include "netcdf/ncVar.h"
#include "netcdf/ncDim.h" 
#include "netcdf/ncAtt.h"
#include "netcdf/ncVarAtt.h"
#include "netcdf/ncCompoundType.h"
#include "netcdf/ncGroupAtt.h"
#include "netcdf/ncGroup.h"
#include "matplot/matplot.h"
#include <string>
#include <algorithm>
#include <iostream>

using JSONValue = crow :: json :: wvalue;
using JSONMap   = crow :: json :: wvalue :: object;
using JSONList  = crow :: json :: wvalue :: list;

using Request   = crow :: request;
using Response  = crow :: response;

using namespace netCDF;
using namespace matplot; 

// JSON and header strings
const std :: string APPLICATION_JSON    =   "application/json";
const std :: string IMAGE_PNG           =   "image/png";
const std :: string NO_CACHE_NO_STORE   =   "no-cache, no-store";

// repeated keys
constexpr char kConcentration[]         =   "concentration";
constexpr char kX[]                     =   "x";
constexpr char kY[]                     =   "y";

constexpr char kError[]                 =   "error";

// Error strings
namespace Errors 
{
    const std :: string UNKNOWN_TYPE    =   "NetCDFServer :: extractMethod: Unknown type";
    const std :: string FAIL_R_NCDF     =   "NetCDFServer :: handleGetInfo: Failed to read NetCDF file: ";
    const std :: string FAIL_O_IMG      =   "NetCDFServer :: handleGetImage: std :: ifstream: failed to open image file. ";
    const std :: string FAIL_STOI       =   "NetCDFServer :: validateRequestParameters: std :: stoi: failed getting parameters. ";
    const std :: string FAIL_S_IMG      =   "NetCDFServer :: generateVisual: Error while saving image: ";
    const std :: string FAIL_START      =   "NetCDFServer :: run: Failed to start server. ";
    const std :: string REMOVE_PARMS    =   "NetCDFServer :: run: remove parms and try again. ";
    const std :: string MISSING_PARMS   =   "NetCDFServer :: validateRequestParameters: Missing required parameters: time and z. ";
    const std :: string INVALID_PARM    =   "NetCDFServer :: validateRequestParameters: Invalid parameter: ";
    const std :: string INDEX_OOR       =   " index out of range - Cannot exceed ";
    const std :: string EXTRACT_NCDF    =   "NetCDFServer :: extractNetCDFSlice: Failed to extract NetCDF data: ";
    const std :: string GRID_EMPTY      =   "NetCDFServer :: generateVisual: Grid data is empty. ";
    const std :: string PNG_TIMEOUT     =   "NetCDFServer :: handleGetImage: Timed out waiting for png visualization. ";
}

class NetCDFServer
{
    public:
        // ctor
        explicit    NetCDFServer( const std :: string& fileName );

        // crow server run method
        void        run         ( uint port = 18080 ); 

        // getters and setters, would go here
        std :: string getFileName()
        {
            return fileName_;
        }

    private:
        // class variables
        const std :: string         fileName_;
        const NcFile                dataFile_;
        static thread_local uint    timeIndex_; 
        static thread_local uint    zIndex_;

        static thread_local uint    responseCode_;

        crow :: SimpleApp           app_;

        // storage for concentration doubles - 
        // will be initialized via resize 
        // once the sizes of x and y are known
        static thread_local std :: vector<double>    concentrationData_;

        // class functions 
        Response    handleGetInfo();
        Response    handleGetData( const Request& request );
        Response    handleGetImage( const Request& request );

        JSONValue   generateVisual( const std :: vector<std :: vector<double>>& grid, 
                                    const std :: string& outputPath ); 

        bool        waitForFile( const std :: string& path, 
                                 int timeoutMs, 
                                 int pollIntervalMs );

        JSONValue   extractNetCDFSlice( int timeIndex, int zIndex );

        void        extractDimensions( JSONValue& result );
        void        extractVariables( JSONValue& result );
        void        extractGlobalAttributes( JSONValue& result );

        bool        validateRequestParameters( const Request& request, 
                                               JSONValue& result,
                                               uint& timeIndex,
                                               uint& zIndex );

        Response    JSONResponse( JSONValue json, std :: string contentType );
};

#endif