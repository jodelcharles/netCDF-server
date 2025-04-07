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

// Error strings
const std :: string ERR_UNKNOWN_TYPE    =   "Unknown type";
const std :: string ERR_FAIL_R_NCDF     =   "Failed to read NetCDF file: ";
const std :: string ERR_FAIL_W_IMG      =   "Failed to generate image.";
const std :: string ERR_FAIL_O_IMG      =   "std :: ifstream: failed to open image file.";
const std :: string ERR_FAIL_S_IMG      =   "Error while saving image: ";
const std :: string ERR_MISSING_PARMS   =   "Missing required parameters: time and z.";
const std :: string ERR_INVALID_PARM    =   "Invalid parameter: ";
const std :: string ERR_INDEX_OOR       =   " index out of range - Cannot exceed ";
const std :: string ERR_EXTRACT_NCDF    =   "Failed to extract NetCDF data: ";
const std :: string ERR_GRID_EMPTY      =   "Error: Grid data is empty.";

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
        const std :: string     fileName_;
        const NcFile            dataFile_;
        uint                    timeIndex_; 
        uint                    zIndex_;

        uint                    responseCode_;

        crow :: SimpleApp       app_;

        std :: vector<double>   concentrationData_;

        const std :: string     imagePath_   =   "assets/concentration_map.png";

        // class functions 
        Response    handleGetInfo();
        Response    handleGetData( const Request& request );
        Response    handleGetImage( const Request& request );

        JSONValue   generateVisual( const std::vector<std::vector<double>>& grid, 
                                    const std::string& outputPath ); 

        JSONValue   extractNetCDFSlice( int timeIndex, int zIndex );

        bool        validateRequestParameters( const Request& request, 
                                               JSONValue& result,
                                               uint& timeIndex,
                                               uint& zIndex );

        Response    JSONResponse( JSONValue json, std :: string contentType );
};

#endif