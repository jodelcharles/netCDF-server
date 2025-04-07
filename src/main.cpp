#include "netcdf_server.h" 

int main() 
{
    NetCDFServer server( "data/concentration.timeseries.nc" );
    server.run();
    return 0;
}

/*--------------------------------------*
| ncdump -c concentration.timeseries.nc |
*---------------------------------------*

netcdf concentration.timeseries {
    dimensions:
        time = 8;
        x = 36;
        y = 27;
        z = 1;
    variables:
        double time(time ) ;
            time:units = "seconds";
            time:long_name = "Seconds since release";
        double y(y ) ;
            y:units = "m";
            y:long_name = "y-coordinate distance from protection point (origin ) ";
        double x(x ) ;
            x:units = "m";
            x:long_name = "x-coordinate distance from protection point (origin ) ";
        double z(z ) ;
            z:units = "m";
            z:long_name = "z-coordinate distance from ground level";
        double concentration(time, z, y, x ) ;
            concentration:units = "kg/m3";
            concentration:long_name = "concentration of agent";
    
     global attributes:
            :id = "1";
            :event_name = "ContinuousEvent";
            :mass_release_rate = "10.0";
            :total_release_time = "600.0";
            :agent = 1;
            :distance_upwind = "5720.0";
            :wind_speed = "5.4";
            :shflx = "60.0";
            :total_mass = "6000.0";
    data:
    
     time = 0.87890625, 879.78516, 1758.6914, 2637.5977, 3516.5039, 4395.4102, 
        5274.3164, 6153.2227;
    
     y = -2444.5, -2256.5, -2068.5, -1880.5, -1692.5, -1504.5, -1316.5, -1128.5, 
        -940.5, -752.5, -564.5, -376.5, -188.5, -0.5, 187.5, 375.5, 563.5, 751.5, 
        939.5, 1127.5, 1315.5, 1503.5, 1691.5, 1879.5, 2067.5, 2255.5, 2443.5;
    
     x = 0, 285.714285714286, 571.428571428571, 857.142857142857, 
        1142.85714285714, 1428.57142857143, 1714.28571428571, 2000, 
        2285.71428571429, 2571.42857142857, 2857.14285714286, 3142.85714285714, 
        3428.57142857143, 3714.28571428571, 4000, 4285.71428571429, 
        4571.42857142857, 4857.14285714286, 5142.85714285714, 5428.57142857143, 
        5714.28571428571, 6000, 6285.71428571429, 6571.42857142857, 
        6857.14285714286, 7142.85714285714, 7428.57142857143, 7714.28571428571, 
        8000, 8285.71428571429, 8571.42857142857, 8857.14285714286, 
        9142.85714285714, 9428.57142857143, 9714.28571428571, 10000;
    
     z = 2;

    .-------------------------------------------------------. 
    | NOTE: in above .nc file, everything is being recorded |
    | at a depth/height of 2m above ground level            | 
    '-------------------------------------------------------'
*/

