//  Created by Futureblur on 25.05.23.

#include "Application.h"

int main(int argc, const char * argv[])
{
    auto args = MG::CommandLineArguments();
    args.count = argc;
    args.list = argv;

    MG::Application::Init(args);
    MG::Application::Run();

    return 0;
}