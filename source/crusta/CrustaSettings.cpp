#include <crusta/CrustaSettings.h>


#include <iostream>

#include <Misc/ConfigurationFile.h>
#include <Misc/File.h>
#include <Misc/StandardValueCoders.h>
#include <GL/GLValueCoders.h>


BEGIN_CRUSTA


CrustaSettings::CrustaSettings() :
    globeName("Sphere_Earth"), globeRadius(6371000.0),
    decoratedVectorArt(false),
    terrainAmbientColor(0.4f, 0.4f, 0.4f, 1.0f),
    terrainDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f),
    terrainEmissiveColor(0.0f, 0.0f, 0.0f, 1.0f),
    terrainSpecularColor(0.3f, 0.3f, 0.3f, 1.0f),
    terrainShininess(55.0f)
{
}

void CrustaSettings::
loadFromFile(std::string configurationFileName, bool merge)
{
    Misc::ConfigurationFile cfgFile;

    //try to load the specified configuration file
    if (!configurationFileName.empty())
    {
        try {
            cfgFile.load(configurationFileName.c_str());
        }
        catch (std::runtime_error e) {
            std::cout << "Caught exception " << e.what() << " while reading " <<
                         "the following configuration file: " <<
                         configurationFileName << std::endl;
        }
    }

    //try to merge the local edits if requested
    if (merge)
    {
        try {
            cfgFile.merge("crustaSettings.cfg");
        }
        catch (Misc::File::OpenError e) {
            //ignore this exception
        }
        catch (std::runtime_error e) {
            std::cout << "Caught exception " << e.what() << " while reading " <<
                         "the local crustaSettings.cfg configuration file." <<
                         std::endl;
        }
    }

    //try to extract misc specifications
    cfgFile.setCurrentSection("/Crusta");
    decoratedVectorArt = cfgFile.retrieveValue<bool>(
        "./decoratedVectorArt", decoratedVectorArt);

    //try to extract the globe specifications
    cfgFile.setCurrentSection("/Crusta/Globe");
    globeName   = cfgFile.retrieveString("./name", globeName);
    globeRadius = cfgFile.retrieveValue<double>("./radius", globeRadius);

    //try to extract the terrain properties
    cfgFile.setCurrentSection("/Crusta/Terrain");
    terrainAmbientColor = cfgFile.retrieveValue<Color>(
        "./terrainAmbientColor", terrainAmbientColor);
    terrainDiffuseColor = cfgFile.retrieveValue<Color>(
        "./terrainDiffuseColor", terrainDiffuseColor);
    terrainEmissiveColor = cfgFile.retrieveValue<Color>(
        "./terrainEmissiveColor", terrainEmissiveColor);
    terrainSpecularColor = cfgFile.retrieveValue<Color>(
        "./terrainSpecularColor", terrainSpecularColor);
    terrainShininess = cfgFile.retrieveValue<double>(
        "./terrainShininess", terrainShininess);
}

END_CRUSTA
