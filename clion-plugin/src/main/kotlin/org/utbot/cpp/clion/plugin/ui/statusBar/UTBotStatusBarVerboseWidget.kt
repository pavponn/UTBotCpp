package org.utbot.cpp.clion.plugin.ui.statusBar

import com.intellij.openapi.project.Project
import com.intellij.openapi.wm.StatusBar
import com.intellij.openapi.wm.StatusBarWidget
import com.intellij.util.Consumer
import org.utbot.cpp.clion.plugin.listeners.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.settings.settings
import java.awt.Component
import java.awt.event.MouseEvent

class UTBotStatusBarVerboseWidget(val project: Project) : StatusBarWidget,
    StatusBarWidget.TextPresentation {
    private var currentStatusBar: StatusBar? = null
    // this connection will be disposed when this widget will be disposed, see VerboseModeWidgetFactory.disposeWidget()
    private val connection = project.messageBus.connect(this)

    init {
        // when user changes verbose mode in settings this listener will update the widget
        connection.subscribe(UTBotSettingsChangedListener.TOPIC, UTBotSettingsChangedListener {
            updateWidget()
        })
    }

    override fun ID(): String = WIDGET_ID

    /**
     * Called when adding widget to statusbar
     *
     * some cases when this happens:
     * - when user enabled widget
     * - status bar was created
     * - StatusBarWidgetsManager.updateWidget was called and this widget must be created
     */
    override fun install(statusbar: StatusBar) {
        this.currentStatusBar = statusbar
        updateWidget()
    }

    private fun updateWidget() {
        currentStatusBar?.updateWidget(ID())
    }

    override fun dispose() {}

    override fun getTooltipText() = VerboseModeWidgetFactory.STATUS_BAR_DISPLAY_NAME

    override fun getClickConsumer() = Consumer<MouseEvent> { _ ->
        val settings = project.settings.storedSettings
        settings.verbose = !settings.verbose
        updateWidget()
    }

    override fun getText(): String {
        return if (project.settings.storedSettings.verbose) "✔ UTBot: verbose formatting" else "❌ UTBot: verbose formatting"
    }

    override fun getAlignment(): Float = Component.CENTER_ALIGNMENT

    override fun getPresentation(): StatusBarWidget.WidgetPresentation = this

    companion object {
        val WIDGET_ID: String = UTBotStatusBarVerboseWidget::class.java.name
    }
}
