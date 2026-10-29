#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
	pluginInstance = p;
	p->addModel(modelCycleClock);
	p->addModel(modelRhythmTalam);
}
