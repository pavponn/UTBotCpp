package org.utbot.cpp.clion.plugin.ui.statusBar

import com.intellij.openapi.project.Project
import com.intellij.openapi.wm.StatusBar
import com.intellij.openapi.wm.StatusBarWidget
import com.intellij.openapi.wm.StatusBarWidgetFactory
import org.utbot.cpp.clion.plugin.settings.settings

class VerboseModeWidgetFactory : StatusBarWidgetFactory {
    override fun getId() = STATUS_BAR_ID

    override fun getDisplayName() = STATUS_BAR_DISPLAY_NAME

    /**
     * Should the widget be created or disposed
     *
     * @return false, then widget will be disposed after StatusBarWidgetsFactory.update call
     * or won't be created at all. true means that widget will be created with [createWidget]
     *
     * @see UTBotStatusBarVerboseWidget's init, where listener to Enable/disable events is created.
     * When user enables/disables plugin, first the setting `isPluginEnabled` changes, then listeners for this TOPIC
     * are called. Listener created from
     */
    override fun isAvailable(project: Project): Boolean {
        return project.settings.storedSettings.isPluginEnabled
    }

    //todo: docs
    override fun createWidget(project: Project): StatusBarWidget {
        return UTBotStatusBarVerboseWidget(project)
    }

    override fun disposeWidget(widget: StatusBarWidget) {
        widget.dispose()
    }

    override fun canBeEnabledOn(statusBar: StatusBar): Boolean = statusBar.project != null

    companion object {
        const val STATUS_BAR_ID = "UTBot: verbose mode"
        const val STATUS_BAR_DISPLAY_NAME = "UTBot: Verbose Formatting"
    }
}
