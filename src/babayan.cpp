#include "babayan.hpp"

int main(int argc, char *argv[]){
    try
    {
        auto vm = config::parse_app_arguments(argc, argv);

        if(vm.first->count("help")){
            std::cout << *vm.second << '\n';
            return EXIT_SUCCESS;
        }

        /* Создать хранилище файлов */
        std::shared_ptr<IKeeper> keeper = std::make_shared<Keeper>();
        
        /* В соответсвии с конфигом, отобрать файлы для сканирования*/
        Scaner scaner(keeper);
        scaner.collect();

        /* Найти дубликаты */
        Reader reader;
        reader.process(keeper);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }   
}
