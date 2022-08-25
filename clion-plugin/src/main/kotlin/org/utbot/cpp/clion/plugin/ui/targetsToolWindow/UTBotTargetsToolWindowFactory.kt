package org.utbot.cpp.clion.plugin.ui.targetsToolWindow

import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.project.Project
import com.intellij.openapi.wm.ToolWindow
import com.intellij.openapi.wm.ToolWindowManager
import org.utbot.cpp.clion.plugin.ui.UTBotAwareToolWindowFactory

class UTBotTargetsToolWindowFactory : UTBotAwareToolWindowFactory() {
    private val logger = Logger.getInstance(this::class.java)

    override fun createToolWindow(project: Project, toolWindow: ToolWindow) {
        logger.info("createToolWindowContent was called")
        val contentManager = toolWindow.contentManager
        val content = contentManager.factory.createContent(
            project.service<UTBotTargetsController>().targetsToolWindow, null, false
        )
        contentManager.addContent(content)
    }
}
