// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/ui/settings/settings_plugins.h"

#include "lang_auto.h"
#include "ayu/ayu_settings.h"
#include "ayu/features/plugins/plugins_engine.h"
#include "ayu/ui/boxes/plugin_info_box.h"
#include "ayu/ui/settings/ayu_builder.h"
#include "ayu/ui/settings/settings_main.h"
#include "core/file_utilities.h"
#include "settings/settings_builder.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "styles/style_widgets.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/popup_menu.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

#include <QtCore/QFile>

namespace Settings {

using namespace Builder;
using namespace AyuBuilder;

namespace {

void BuildPluginsList(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addSkip();
	builder.addSubsectionTitle(tr::ayu_PluginsInstalledList());

	builder.add([=](const WidgetContext &ctx) -> SectionBuilder::WidgetToAdd {
		auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
		const auto container = wrap.data();

		const auto fillList = [=] {
			auto children = container->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
			for (auto *w : children) {
				delete w;
			}
			const auto &plugins = AyuSettings::getInstance().installedPlugins();
			if (plugins.empty()) {
				container->add(
					object_ptr<Ui::FlatLabel>(
						container,
						tr::ayu_NoPluginsInstalled(),
						st::boxLabel),
					st::boxRowPadding);
			} else {
				for (const auto &[id, plugin] : plugins) {
					const auto statusStr = plugin.enabled ? u""_q : tr::lng_settings_auto_night_disabled(tr::now);
					const auto label = plugin.name + u" (v"_q + plugin.version + u")"_q + (statusStr.isEmpty() ? u""_q : u" - "_q + statusStr);
					const auto button = container->add(
						object_ptr<Ui::SettingsButton>(
							container,
							rpl::single(label),
							st::settingsButtonNoIcon));
					if (!plugin.enabled) {
						button->setColorOverride(st::storiesComposeGrayText->c);
					}
					button->setClickedCallback([=, id = plugin.id] {
						auto *menu = new Ui::PopupMenu(button, st::popupMenuWithIcons);
						menu->setAttribute(Qt::WA_DeleteOnClose);

						menu->addAction(
							tr::lng_info_about(tr::now),
							[=] {
								const auto map = AyuSettings::getInstance().installedPlugins();
								const auto it = map.find(id);
								if (it != map.end()) {
									Ui::PluginMetadata md;
									md.id = it->second.id;
									md.name = it->second.name;
									md.description = it->second.description;
									md.author = it->second.author;
									md.version = it->second.version;
									md.icon = it->second.icon;
									for (const auto &req : it->second.requirements.split(',')) {
										const auto trimmed = req.trimmed();
										if (!trimmed.isEmpty()) md.requirements.append(trimmed);
									}
									Ui::ShowPluginInfoBox(controller, it->second.filePath, md);
								}
							},
							&st::menuIconInfo);

						const auto map = AyuSettings::getInstance().installedPlugins();
						const auto it = map.find(id);
						const auto isEnabled = (it != map.end()) && it->second.enabled;

						menu->addAction(
							isEnabled ? tr::lng_settings_auto_night_disable(tr::now) : tr::lng_sure_enable(tr::now),
							[=] {
								AyuSettings::getInstance().setPluginEnabled(id, !isEnabled);
							},
							isEnabled ? &st::menuIconBlock : &st::menuIconUnblock);

						menu->addSeparator();

						menu->addAction(
							tr::lng_theme_delete(tr::now),
							[=] {
								const auto map2 = AyuSettings::getInstance().installedPlugins();
								const auto it2 = map2.find(id);
								if (it2 != map2.end() && QFile::exists(it2->second.filePath)) {
									QFile::remove(it2->second.filePath);
								}
								AyuSettings::getInstance().uninstallPlugin(id);
							},
							&st::menuIconDelete);

						menu->popup(QCursor::pos());
					});
				}
			}
		};

		fillList();
		AyuSettings::getInstance().pluginsChanged() | rpl::on_next([=] {
			fillList();
		}, container->lifetime());

		return { .widget = std::move(wrap), .align = style::al_top };
	});

	builder.addSkip();
	builder.addButton({
		.id = u"ayu/openPluginsFolder"_q,
		.title = tr::ayu_OpenPluginsFolder(),
		.icon = { &st::menuIconShowInFolder },
		.onClick = [=] {
			File::ShowInFolder(AyuSettings::pluginsDirectory());
		},
	});
	builder.addSkip();
	builder.addDividerText(AyuFeatures::Plugins::isPythonAvailable()
		? tr::ayu_PythonStatusFound()
		: tr::ayu_PythonStatusNotFound());
}

const auto kMeta = BuildHelper({
	.id = AyuPlugins::Id(),
	.parentId = AyuMain::Id(),
	.title = &tr::ayu_CategoryPlugins,
	.icon = &st::menuIconBotCommands,
}, [](SectionBuilder &builder) {
	auto ayu = AyuSectionBuilder(builder);

	builder.addSkip();
	ayu.addSettingToggle({
		.id = u"ayu/pluginsEnabled"_q,
		.title = tr::ayu_PluginsEnabled(),
		.getter = &AyuSettings::pluginsEnabled,
		.setter = &AyuSettings::setPluginsEnabled,
		.icon = { &st::menuIconBotCommands },
	});
	builder.addSkip();
	builder.addDividerText(tr::ayu_PluginsDescription());

	BuildPluginsList(builder);
});

} // namespace

rpl::producer<QString> AyuPlugins::title() {
	return tr::ayu_CategoryPlugins();
}

AyuPlugins::AyuPlugins(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

void AyuPlugins::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	build(content, kMeta.build);
	Ui::ResizeFitChild(this, content);
}

Type AyuPluginsId() {
	return AyuPlugins::Id();
}

} // namespace Settings
