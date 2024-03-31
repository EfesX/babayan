#define BOOST_TEST_MODULE keeper_test

#include <boost/test/unit_test.hpp>

#include "babayan.hpp"

BOOST_AUTO_TEST_SUITE(keeper_test)



BOOST_AUTO_TEST_CASE(test_1)
{
    std::shared_ptr<IKeeper> keeper = std::make_shared<Keeper>();
    
    keeper->add_file(boost::filesystem::path("file1"), 2);
    keeper->add_file(boost::filesystem::path("file1"), 3);
    keeper->add_file(boost::filesystem::path("file3"), 4);
    keeper->add_file(boost::filesystem::path("file4"), 5);
    keeper->add_file(boost::filesystem::path("file5"), 6);

    int cnt = 0;
    for (auto g : keeper->group_by_size()){
        cnt++;
    }

    BOOST_ASSERT(cnt == 4);

    keeper->add_file(boost::filesystem::path("file11"), 2);
    keeper->add_file(boost::filesystem::path("file12"), 3);
    keeper->add_file(boost::filesystem::path("file2"), 3);
    keeper->add_file(boost::filesystem::path("file8"), 4);
    keeper->add_file(boost::filesystem::path("file9"), 5);
    keeper->add_file(boost::filesystem::path("file10"), 6);



    cnt = 0;
    for (auto g : keeper->group_by_size()){
        while(g.first != g.second){
            std::cout << g.first->path << "  " << g.first->size << std::endl;
            cnt++;
            g.first++;
        }
        cnt--;
        cnt--;
    }

    BOOST_ASSERT(cnt == 0);
}


BOOST_AUTO_TEST_SUITE_END()
