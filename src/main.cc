
#include <thread>
#include <csignal>
#include <memory>

#include "nexran.h"
#include "restserver.h"

static int signaled = -1;
static std::unique_ptr<nexran::App> xapp;

void sigh(int signo) {
    signaled = signo;
    xapp->stop();
    exit(0);
}

int main(int argc,char **argv) {
    std::thread::id this_id;
    std::unique_ptr<nexran::Config> config;

    this_id = std::this_thread::get_id();

    config = std::make_unique<nexran::Config>();
    if (!config || !config->load()) {
	//mdclog_write(MDCLOG_ERROR,"Failed to load config");
	exit(1);
    }

    xapp = std::make_unique<nexran::App>(*config);

    signal(SIGINT,sigh);
    signal(SIGHUP,sigh);
    signal(SIGTERM,sigh);

    xapp->init();
    xapp->start();

    while (true)
	sleep(1);

    exit(0);
}
