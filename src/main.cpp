/*
 * Upload (stage) a single large image or directory tree using multiple threads
 */
#include <fstream>
#include <string>
#include <memory>
#include <map>
#include <unordered_set>
#include <argparse.h>

//#include <netcode/aws/s3/S3Util.h>
//#include <netcode/aws/s3/S3HeadRequest.h>

#include <MathsLibrary.h>
#include <RoamesLogger.h>
#include <RoamesError.h>
//#include <S3IDZipCompressor.h>
#include <CLMainWrapper.h>
#include <CLParamValidator.h>
#include <tinyformat.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

using namespace Roames;
using namespace std;

class MbeFilePairing
{
    vector<string> m_testlistFiles;
    string m_suffix1;
    string m_suffix2;

public:
    MbeFilePairing() 
    {
        m_suffix1 = "port-mbe.xyz";
        m_suffix2 = "starbd-mbe.xyz";
        
        m_testlistFiles.push_back("dir/sub/test012_port-MBE.xyz");
        m_testlistFiles.push_back("dir/sub/test123_Port-MBE.xyz");
        m_testlistFiles.push_back("dir/sub/test234_port-MBE.xyz");
        m_testlistFiles.push_back("dir/sub/test123_starbd-MBE.xyz");
        m_testlistFiles.push_back("dir/sub/test234_Starbd-MBE.xyz");
        m_testlistFiles.push_back("dir/sub/test896_starbd-MBE.xyz");
        
     };


    // returns index of paired file; -1 if none
    // **Is case-sensitive.
    static int findPairedFile(const string& file1, const vector<string>& listFiles, const string& suffix1, const string& suffix2)
    {
      
        uint compareLength = 0;
        string compareSuffix = "";
        string root = "";

        // Does our file contain suffix1?
        if ((file1.size() > suffix1.size()) &&
                ! file1.compare(file1.size()-suffix1.size(), suffix1.size(), suffix1))
        {
            root = file1.substr(0,file1.size()-suffix1.size());
            compareLength = suffix2.size();
            compareSuffix = suffix2;
        }
        else if ((file1.size() > suffix2.size()) &&
                ! file1.compare(file1.size()-suffix2.size(), suffix2.size(), suffix2))
        {
            root = file1.substr(0,file1.size()-suffix2.size());
            compareLength = suffix1.size();
            compareSuffix = suffix1;
        }
        else
            return -1;  // Can't find a match if we can't match a suffix


        for (size_t i = 0; i < listFiles.size(); i++)
        {
            // compare the root section of all files in the list
            const string& file2 = listFiles[i];
            if (! file2.compare(0, root.size(), root))
            {
                // check that the pairing suffix matches
                if ((file2.size() > compareLength) &&
                        ! file2.compare(file2.size()-compareSuffix.size(), compareSuffix.size(), compareSuffix))
                {
                    // this might be the exact same file; don't return that one.
                    if (file1.compare(file2))
                    {
                        return i;  // found a pairing
                    }
                }
            }  
        }
        return -1;
    }

    // returns a vector of string pairs separated by space
    // **Not case-sensitive
    static vector<string> generatePairedFilesList(const vector<string>& originalFileList, const string& originalSuffix1, const string& originalSuffix2)
    {
        // keep track of which files have been handled
        unordered_set<int> indices;

        // create case-insensitive dataset
        vector<string> sourceFileList;
        string suffix1 = originalSuffix1;
        string suffix2 = originalSuffix2;
        
        // make lowercase
        std::transform(suffix1.begin(), suffix1.end(), suffix1.begin(), ::tolower);
        std::transform(suffix2.begin(), suffix2.end(), suffix2.begin(), ::tolower);

        int originalLength = originalFileList.size();
        for(int i = 0; i < originalLength; i++)
        {
            indices.insert(i);
            string temporary = originalFileList[i];
            std::transform(temporary.begin(), temporary.end(), temporary.begin(), ::tolower);
            sourceFileList.push_back(temporary);
        }
        
                // build the output string list
        vector<string> foundList;

        // build a set with all the file indices; pull them as the pairs are found
        unordered_set<int>::iterator ii = indices.begin();
        while (ii != indices.end())
        {
            int found = -1;
            if ((found = findPairedFile(sourceFileList[*ii], sourceFileList, suffix1, suffix2)) > -1)
            {
                // pair found; build a space-separated string with both filenames
                ostringstream pairString;
                pairString << sourceFileList[*ii] << ' ' << sourceFileList[found];
                foundList.push_back(pairString.str().c_str());

                // remove the found index from our set
                unordered_set<int>::iterator iiPair = indices.find(found);
                if (iiPair == indices.end())
                {
                    throw("Unknown exception: FNF"); 
                }
                else if (iiPair == ii)
                {
                    throw("Unknown exception: FFS");
                }
                else
                {
                    indices.erase(indices.find(found));
                }
            }
            else
            {
                // pair not found; just take the single file
                foundList.push_back(sourceFileList[*ii]);
            }

            // remove this(current) index from the set
            ii = indices.erase(ii);
        }

        return foundList;
    }

    // returns 0 if passes, otherwise -1
    int testFindPairedFiles()
    {
        int result=0;

        result = min(result, testFindPairedFile(1,3));
        result = min(result, testFindPairedFile(3,1));
        result = min(result, testFindPairedFile(4,2));
        result = min(result, testFindPairedFile(2,4));
        result = min(result, testFindPairedFile(0,-1));
        result = min(result, testFindPairedFile(5,-1));

        if (result < 0)
        {
            LOG_ERROR("TestFindPairedFiles failed.");
            return result;
        }

        cout << "TestFindPairedFiles passed\n";
        return 0;
    }
    
    int testReturnStringList()
    {
        cout << "\n";
        cout << "Finding pairs for this list:\n";
        for (vector<string>::iterator it = m_testlistFiles.begin(); it != m_testlistFiles.end(); it++)
        {
            cout << *it << "\n";
        }


        vector<string> resultList = generatePairedFilesList(m_testlistFiles, m_suffix1, m_suffix2);
        
        cout << "\n";
        cout << "Found pairs:\n";
        for (vector<string>::iterator it = resultList.begin(); it != resultList.end(); it++)
        {
            cout << *it << "\n";
        }

        int length = resultList.size();
        if (length != 4)
        {
            LOG_ERROR("TestReturnStringList failed");
            return -1;
        }

        cout << "TestReturnStringList passed\n";
        return 0;
    }

    // runs all tests
    int testAll()
    {
        int result = 0;
        result = min(result, testFindPairedFiles());
        result = min(result, testReturnStringList());

        if (result < 0)
            LOG_ERROR("Test run failed.");

        return result;
    }

private:

    int testFindPairedFile(int base, int expectedPair)
    {
        // create case-insensitive dataset
        vector<string> sourceFileList;
        string suffix1 = m_suffix1;
        string suffix2 = m_suffix2;
        
        // make lowercase
        std::transform(suffix1.begin(), suffix1.end(), suffix1.begin(), ::tolower);
        std::transform(suffix2.begin(), suffix2.end(), suffix2.begin(), ::tolower);

        int originalLength = m_testlistFiles.size();
        for(int i = 0; i < originalLength; i++)
        {
            string temporary = m_testlistFiles[i];
            std::transform(temporary.begin(), temporary.end(), temporary.begin(), ::tolower);
            sourceFileList.push_back(temporary);
        }
        
        int result = findPairedFile(sourceFileList[base], sourceFileList, suffix1, suffix2);
        if (result != expectedPair)
        {
            ostringstream message;
            message << "TestFindPairedFile(" << base << "," << expectedPair << ") returned wrong result: " << result;
            const char* szMessage = message.str().c_str();
            LOG_ERROR(szMessage);
            return -1;
        }

        return 0;
    }

};


class POSHeader
{
public:
    POSHeader() {}
    virtual ~POSHeader() {}

    void read(std::istream &is);

    const size_t getDataSize() const { return m_c_filesize; }
    const size_t getDataOffset() const { return m_dataOffset; }

    const std::string & getName() const { return m_fileName; }

protected:
    static const uint8_t ms_magic[6];   // must be "070707"

    std::string m_fileName;
    uint16_t m_c_dev;          //  6
    uint16_t m_c_ino;          //  6
    uint16_t m_c_mode;         //  6       see below for value
    uint16_t m_c_uid;          //  6
    uint16_t m_c_gid;          //  6
    uint16_t m_c_nlink;        //  6
    uint16_t m_c_rdev;         //  6       only valid for chr and blk special files
    uint32_t m_c_mtime;        //  11
    uint16_t m_c_namesize;     //  6       count includes terminating NUL in pathname
    uint32_t m_c_filesize;     //  11      must be 0 for FIFOs and directories
    size_t m_headerOffset;
    size_t m_dataOffset;
};


const uint8_t POSHeader::ms_magic[6] = {0x30, 0x37, 0x30, 0x37, 0x30, 0x37}; // 070707

class POSFile
{
public:
    POSFile(const std::string &filename) : m_filename(filename)
    {
        open(filename);
    }
    virtual ~POSFile()
    {
        close();
    }

    void open(const std::string & filename)
    {
        if (m_fileStream.is_open())
        {
            throw RoamesError("Input file stream is already open");
        }
        if (filename.empty())
        {
            throw RoamesError("Empty filename provided for CPIO open");
        }
        m_fileStream.open(filename, std::ifstream::in|std::ifstream::binary);
        if (!m_fileStream.is_open())
        {
            throw RoamesError("Input file stream could not be opened");
        }
    }

    void close()
    {
        if (m_fileStream.is_open())
        {
            m_fileStream.close();
        }
    }

    int read()
    {
        if (!m_fileStream.is_open())
        {
            throw RoamesError("CPIO archive is not open");
        }

        while (m_fileStream)
        {
            try
            {
                POSHeader hdr;
                hdr.read(m_fileStream);
                m_fileStream.seekg(hdr.getDataOffset() + hdr.getDataSize());
                if (hdr.getName() == "TRAILER!!!")  // the last entry should always be a mock file called TRAILER!!!
                {
                    break;
                }
                m_headers[hdr.getName()] = hdr;
            }
            catch (std::exception &e)
            {
                throw RoamesError("FAILED: Could not read file header information from cpio archive");
            }
        }

        return m_headers.size() < 1 ? 0 : m_headers.size()-1;
    }

private:
    typedef std::map<std::string, POSHeader> HeaderMap;

    HeaderMap m_headers;
    std::string m_filename;
    std::ifstream m_fileStream;
};

/*
class SfxMBEData
{
public:
    SfxMBEData() : m_time(0.0), m_pingCounter(0), m_intensity(0) {}
    virtual ~SfxMBEData() {}

private:
    double m_time;
    int m_pingCounter;
    short m_intensity;

    MathsLibrary::Vector3D m_enhPos;    // Easting/Northing/Height position
    MathsLibrary::Vector3D m_llePos;    // Lat/Long/Elevation position
    MathsLibrary::Vector3D m_mbePos;    // MBE xyz data
};
*/




// Size of chunks which are written to laser disks
// 1024 * 1024 * 200 == ~200MiB per part
const int g_partSize = 209715200;

// TODO: Pass `simulate` through to multiPartUpload() and uploadDirectoryTree(), to perform a proper simulation of everything.


std::vector<std::string> g_positionalArgs;
static int parsePositionalArgs(int argc, const char* argv[])
{
    assert(argc == 1);
    g_positionalArgs.push_back(argv[0]);
    return 0;
}

// TODO: move this complex test suite to boost
void RunTest()
{
    // Call test class
    MbeFilePairing fixture;
    fixture.testAll();
}

// Handle command line args
int roamesSfxPOSReaderMain(int argc, const char* argv[])
{

    namespace fs = boost::filesystem;

    const char* versionStr = "Roames SfxPOSReader Version: 0.0.1 (First blood)";

    ArgParse::ArgParse ap;

    bool printHelp = false;

    std::string xyzFile;

    int numThreads = 1;

    int logLevel = -1;

    ap.options (
        "SfxPOSReader - Extraxt XYZ data from StarFix MBE pos assets into S3\n"
        "Usage:  SfxPOSReader <TBA> <Input Files> -xyzfile <Output XYZ File>\n"
        "\n"
        "Exit Codes:\n"
        "     0: No errors (Program succeeded)\n"
        "     1: General error\n"
        "     2: Command line usage error",

        "%*",               &parsePositionalArgs,   "", // output path

        "<SEPARATOR>", "\nOne of the following actions must be specified:",
        "-xyzfile %s",     &xyzFile,                "The number of bytes to copy",

        "<SEPARATOR>", "\nThe following parameters may also be specified:",
        "-numthreads %d",   &numThreads,            "How many upload threads to use [default = 1].",

        "<SEPARATOR>", "\nDebugging options:",
        "-loglevel %d",     &logLevel,              "Logging verbosity (default = 3 (log_info))",
        "-help",            &printHelp,             "Print this help message",

        NULL
    );

    if (argc < 2 || ap.parse(argc, argv) < 0)
    {
        ap.usage();
        std::string errMessage = ap.geterror();
        if (errMessage.empty())
            std::cout << std::endl << versionStr << std::endl << std::endl;
        else
            std::cout << "\n[ERROR] " << errMessage << std::endl;
        return PROGRAM_USAGE_ERROR;
    }
    else if (printHelp)
    {
        ap.usage();
        return PROGRAM_EXIT_SUCCESS;
    }

    if ((logLevel < 0) || (logLevel > 10))
    {
        logLevel = RoamesLogger::Info;
    }
    RoamesLogger::instance().setLogLevel(static_cast<RoamesLogger::LogLevel>(logLevel));

    fs::path sourcePath;
    std::string outputPath;

    // Check usage
    try
    {
        if (g_positionalArgs.size() != 2)   // ick, but for now we must only have 1 in and 1 out
        {
            if (g_positionalArgs.size() < 1)    // no args
            {
                throw RoamesError("At least one input path is required.");
            }
        }

        sourcePath = g_positionalArgs[0];
        // check for recursive or number of bytes.  If neither then assume num bytes and attempt to determine size
        if (xyzFile.empty())
        {
            throw RoamesError("No xyz file name provided");
        }

        outputPath = xyzFile;

        // -----------------------------------------------------
        //     check required input
        // -----------------------------------------------------


        ///-----------------------------------------------------
        ///     optional argument checks
        ///-----------------------------------------------------

        if (numThreads < 1)
        {
            throw RoamesError("The -numthread parameter needs to be specified and greater than zero");
        }
    }
    catch(RoamesError& e)
    {
        LOG_ERROR("Roames Error: %s", e.what());
        return PROGRAM_USAGE_ERROR;
    }
    catch (boost::bad_lexical_cast const& e)
    {
        LOG_ERROR("An exception occured while converting numerical arguments: %s", e.what());
        return PROGRAM_USAGE_ERROR;
    }
    catch (std::exception& e)
    {
        LOG_ERROR("A standard exception occured while processing the command line: %s", e.what());
        return PROGRAM_USAGE_ERROR;
    }
    catch(...)
    {
        LOG_ERROR("An unknown exception occured while parsing arguments.");
        return PROGRAM_USAGE_ERROR;
    }

    // Process POS
    try
    {
        tfm::printf("DOOOOOOOOM\n");
        std::ifstream inFile(g_positionalArgs[0], std::ifstream::in|std::ifstream::binary);

        if (!inFile.is_open())
        {
            LOG_ERROR("Could not open input file \"%s\"", g_positionalArgs[0]);
            return PROGRAM_EXIT_ERROR;
        }

        /*
        POSHeader hdr, hdr2, hdr3;
        hdr.read(inFile);
        inFile.seekg(hdr.getDataOffset() + hdr.getDataSize());
        hdr2.read(inFile);
        inFile.seekg(hdr2.getDataOffset() + hdr2.getDataSize());
        hdr3.read(inFile);
        /*/
        POSFile fl(g_positionalArgs[0]);
        fl.read();
        //*/
    }
    catch(CommandLineOptionError& e)
    {
        LOG_ERROR("CLI %s", e.what());
        return PROGRAM_EXIT_ERROR;
    }
    catch(RoamesError& e)
    {
        LOG_ERROR("RE %s", e.what());
        return PROGRAM_EXIT_ERROR;
    }
    catch(std::exception& e)
    {
        LOG_ERROR("stde %s", e.what());
        return PROGRAM_EXIT_ERROR;
    }
    catch(...)
    {
        LOG_ERROR("An unknown exception occured");
        return PROGRAM_EXIT_ERROR;
    }

    return PROGRAM_EXIT_SUCCESS;
}

CL_MAIN_WRAP(roamesSfxPOSReaderMain)

template <typename T>
void readOctBuffer(std::istream & is, char * buf, const size_t bufSize, T &value)
{
    if (!is)
    {
        throw RoamesError("Invalid stream");
    }
    if (!buf || bufSize <= 6)    // HACK :(
    {
        throw RoamesError("Invalid Buffer");
    }
    size_t readSize = bufSize-1;
    is.read(buf, readSize);
    if (!is || (is.gcount() != readSize))
    {
        throw RoamesError("Could not read %u byte buffer", readSize);
    }
    else
    {
        value = 0;
        std::stringstream ss(buf);
        ss.setf(std::ios::oct, std::ios::basefield);
        ss >> value;

        LOG_DEBUG("Read string \"%s\" value = %u", buf, value);
    }
}

void POSHeader::read(std::istream & is)
{
    if (!is)
    {
        throw RoamesError("Invalid input stream");
    }

    char small_buf[7] = {0};    // 6 chars and null
    char large_buf[12] = {0};    // 11 chars and null

    memset(small_buf, 0, 7);
    memset(large_buf, 0, 12);

    m_headerOffset = is.tellg();

    // read the header
    // first check the magic value
    is.read(small_buf, 6);
    if (!is || (is.gcount() != 6))
    {
        throw RoamesError("Failed to read small buffer");
    }
    if (memcmp(small_buf, ms_magic, 6) != 0)
    {
        throw RoamesError("Could not read CPIO header magic value");
    }

    readOctBuffer<uint16_t>(is, small_buf, 7, m_c_dev);
    readOctBuffer<uint16_t>(is, small_buf, 7, m_c_ino);
    readOctBuffer<uint16_t>(is, small_buf, 7, m_c_mode);
    readOctBuffer<uint16_t>(is, small_buf, 7, m_c_uid);
    readOctBuffer<uint16_t>(is, small_buf, 7, m_c_gid);
    readOctBuffer<uint16_t>(is, small_buf, 7, m_c_nlink);
    readOctBuffer<uint16_t>(is, small_buf, 7, m_c_rdev);
    readOctBuffer<uint32_t>(is, large_buf, 12, m_c_mtime);
    readOctBuffer<uint16_t>(is, small_buf, 7, m_c_namesize);
    readOctBuffer<uint32_t>(is, large_buf, 12, m_c_filesize);

    if (m_c_namesize >= 1)
    {
        char nameBuf[1024];
        memset(nameBuf, 0, 1024);
        is.read(nameBuf, m_c_namesize);
        if (!is || (is.gcount() != m_c_namesize))
        {
            throw RoamesError("Could not read file name");
        }
        m_fileName = nameBuf;
        LOG_DEBUG("Filename = \"%s\"", m_fileName);
    }
    else
    {
        throw RoamesError("Invalid name size");
    }

    m_dataOffset = m_headerOffset + 76 + m_c_namesize;
    LOG_DEBUG("Offsets: start = %u, data = %u", m_headerOffset, m_dataOffset);
}


