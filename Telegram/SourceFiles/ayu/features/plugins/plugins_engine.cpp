// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/plugins/plugins_engine.h"

#include "ayu/ayu_settings.h"
#include <QtCore/QFile>
#include <QtCore/QProcess>

namespace AyuFeatures::Plugins {

bool isPythonAvailable() {
	QProcess process;
	process.start(u"python"_q, { u"--version"_q });
	if (!process.waitForFinished(2000)) {
		process.start(u"python3"_q, { u"--version"_q });
		return process.waitForFinished(2000) && (process.exitStatus() == QProcess::NormalExit);
	}
	return (process.exitStatus() == QProcess::NormalExit);
}

QString pythonVersion() {
	QProcess process;
	process.start(u"python"_q, { u"--version"_q });
	if (!process.waitForFinished(2000)) {
		process.start(u"python3"_q, { u"--version"_q });
		if (!process.waitForFinished(2000)) {
			return u"Not Found"_q;
		}
	}
	auto out = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
	if (out.isEmpty()) {
		out = QString::fromUtf8(process.readAllStandardError()).trimmed();
	}
	return out.isEmpty() ? u"Unknown"_q : out;
}

void start() {
	const auto &settings = AyuSettings::getInstance();
	if (!settings.pluginsEnabled()) {
		return;
	}
	for (const auto &[id, plugin] : settings.installedPlugins()) {
		if (plugin.enabled && QFile::exists(plugin.filePath)) {
			auto *process = new QProcess();
			QObject::connect(process, Fn<void(int, QProcess::ExitStatus)>(
				[process](int, QProcess::ExitStatus) {
					process->deleteLater();
				}));
			process->start(u"python"_q, { plugin.filePath });
		}
	}
}

} // namespace AyuFeatures::Plugins
