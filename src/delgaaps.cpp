#include "../include/main.hpp"

// Помощь по параметрам командной строки
void DelGapps::show_help()
{
    fprintf(stderr, R"EOF(
delgaaps

Usage:

   delgaaps <folder> <file_list>
        Where:
        <folder> - path to the folder where the image was unpacked
        <file_list> - a file with a list to remove gapps and more
            if <file_list> is not specified it will be used by default
        For example:
        delgaaps "/sdcard/Unpacker" "/sdcard/gappslist.txt"

)EOF");
    fprintf(stderr, "\n");
}
void default_file_list(const std::filesystem::path& path){
    std::vector<std::string> standart_list = {
            "AnalyticsCore",
            "BasicDreams",
            "BookmarkProvider",
            "CloudService",
            "EasterEgg",
            "facebook-appmanager",
            "Joyose",
            "LiveWallpapersPicker",
            "MiCloudSync",
            "MiConnectService",
            "MIDrop",
            "miui",
            "MiuiBugReport",
            "MIUIHealthGlobal",
            "MIUIMiPicks",
            "MIUINotes",
            "MIUISystemAppUpdater",
            "PaymentService",
            "Stk",
            "Updater",
            "XiaomiAccount",
            "XiaomiSimActivateService",
            "XMRemoteController",
            "CloudBackup",
            "facebook-installer",
            "facebook-services",
            "FindDevice",
            "MIService",
            "MIShareGlobal",
            "MIUIGlobalMinusScreenWidget",
            "MIUIWeatherGlobal",
            "MIUIYellowPageGlobal",
            "Tag",
            "GoogleOne",
            "GooglePay",
            "Gmail2",
            "Chrome",
            "Maps",
            "PhotoTable",
            "SpeechServicesByGoogle",
            "talkback",
            "YouTube",
            "wps_lite",
            "AndroidAutoStub",
            "GoogleAssistant",
            "HotwordEnrollmentXGoogleHEXAGON",
            "HotwordEnrollmentOKGoogleHEXAGON",
            "Turbo",
            "Velvet",
            "Wellbeing",
            "MipayService",
            "GoogleFeedback",
            "MiGameCenterGlobal"
    };
    if (std::filesystem::exists(path))
    {
        //write vector to file -->
        std::fstream config(path, std::ios::out);
        std::copy(standart_list.begin(), standart_list.end(), std::ostream_iterator<std::string>(config, "\n"));
        //write vector to file <--
    }
}
// Парсить командную строку
ParseResult DelGapps::parse_cmd_line(int argc, char* argv[])
{
    if(argc < 1 || argc > 2 ){show_help();}
    if(argc == 2){
        folder_path = argv[0];
        file_path = argv[1];
    }
    if(argc == 1){
        folder_path = argv[0];
        if(std::filesystem::exists("/sdcard")){
           default_list = "/sdcard/default_list.txt";
        }else{
            default_list = "./default_list.txt";
        }
        default_file_list(default_list);
        file_path = default_list;
    }
    return ParseResult::ok;
}

// Основная функция
ProcessResult DelGapps::process()
{
    std::cout << "\nDesigned by blackeangel (blackeangel@mail.ru) for UKA tools" << std::endl;
    std::cout << "delgaaps v.1.0.1\n"
              << std::endl;

    if (!std::filesystem::exists(folder_path))
    {
        std::cout << folder_path << " not found!" << std::endl;
        exit(2);
    }
    if (!std::filesystem::exists(file_path))
    {
        std::cout << file_path << " not found! Used default list" << std::endl;
        //exit(2);
    }

    std::string config_path;
    std::string config_file;
    std::string name_img_path;
    std::vector<std::string> name_img;
    std::string gaapslist_file(file_path.string());
    std::vector<std::string> gaapslist = readlines(gaapslist_file);
    std::vector<std::string> del_list;
    std::filesystem::path tmppath;

    //analize root dir -->
    std::vector<std::string> root_dir_list = list_dir(folder_path);
    auto config_iter = std::find(root_dir_list.begin(), root_dir_list.end(), "config");
    if (config_iter != root_dir_list.end())
    {
        tmppath = folder_path;
        tmppath /= *config_iter;
        config_path = tmppath.string();
        root_dir_list.erase(config_iter);

        for (auto &&root_dir : root_dir_list)
        {
            tmppath = config_path;
            tmppath /= root_dir;
            tmppath /= root_dir + "_fs_config";
            config_file = tmppath.string();
            std::vector<std::string> tmp = readlines(config_file);
            name_img.insert(name_img.end(), tmp.begin(), tmp.end());
        }
    }
    else
    {
        std::cout << "Config file not found! Will be configurated from files...\n"
                  << std::endl;
        for (auto &&root_dir : root_dir_list)
        {
            tmppath = folder_path;
            tmppath /= root_dir;
            root_dir = tmppath.string();
        }
        name_img = recursive_dir(root_dir_list);
    }
    //analize root dir <--
    std::vector<std::string> name_img_clone = name_img;
    std::regex reg(R"( \d{1,4} \d{1,4} \d{1,4}( .*)?)", std::regex_constants::optimize | std::regex_constants::ECMAScript);
    for (auto & i : name_img_clone)
    {
        i = regex_replace(i, reg, "");
    }
    int pos;
    for (const auto & i : name_img_clone)
    {
        std::string s1 = i;
        std::transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
        for (auto s2 : gaapslist)
        {
            std::transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
            pos = s1.find(s2, 0);
            if (pos != std::string::npos)
            {
                del_list.push_back(i);
                break;
            }
        }
    }
    std::copy(del_list.begin(), del_list.end(), std::ostream_iterator<std::string>(std::cout, "\n"));

    std::cout << "\nAre you sure you want to delete these files/folders?(Yes or No)" << std::endl;
    std::string answer;
    std::cin >> answer;
    if (answer == "y" || answer == "yes" || answer == "Y" || answer == "Yes")
    {
        for (int i = 0; i < del_list.size(); i++)
        {
            std::filesystem::path path = folder_path;
            path /= name_img_clone[i];
            if (std::filesystem::exists(path))
            {
                try
                {
                    std::filesystem::remove_all(del_list[i]);
                    std::cout << "Deleted: " << del_list[i] << std::endl;
                }
                catch (std::filesystem::filesystem_error &e)
                {
                    std::cout << "Can't remove " << del_list[i] << "! Missed..." << std::endl;
                }
            }
        }
        if (!config_path.empty())
        {
            for (auto &&root_dir : root_dir_list)
            {
                tmppath = config_path;
                tmppath /= root_dir;
                tmppath /= root_dir + "_fs_config";
                config_file = tmppath.string();
                std::vector<std::string> tmp = readlines(config_file);
                for (auto & j : tmp)
                {
                    j = regex_replace(j, reg, "");
                    for (const auto & k : del_list)
                    {
                        if (j == k)
                        {
                            j = "";
                        }
                    }
                }
                //delete empty elements from vector -->
                auto isEmptyOrBlank = [](const std::string &s) {
                    return s.find_first_not_of("") == std::string::npos;
                };
                tmp.erase(std::remove_if(tmp.begin(), tmp.end(), isEmptyOrBlank), tmp.end());
                //delete empty elements from vector <--

                //write vector to file -->
                std::fstream config(config_file, std::ios::out);
                std::copy(tmp.begin(), tmp.end(), std::ostream_iterator<std::string>(config, "\n"));
                //write vector to file <--
            }
        }
    }
    else
    {
        exit(0);
    }

    return ProcessResult::ok;
}
