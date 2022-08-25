package org.utbot.cpp.clion.plugin.ui.userLog

import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.project.Project
import com.intellij.openapi.wm.ToolWindow
import org.utbot.cpp.clion.plugin.ui.UTBotAwareToolWindowFactory

class ConsoleToolWindowProvider : UTBotAwareToolWindowFactory() {
    private val logger = Logger.getInstance(this::class.java)

    override fun createToolWindow(project: Project, toolWindow: ToolWindow) {
        logger.debug("createToolWindowContent was called")

        val contentManager = toolWindow.contentManager
        val content =
            contentManager.factory.createContent(ConsoleToolWindow(project), null, false)
        contentManager.addContent(content)
    }
}
