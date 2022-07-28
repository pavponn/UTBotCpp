package org.utbot.cpp.clion.plugin.ui.targetsToolWindow

import com.intellij.openapi.project.Project
import com.intellij.ui.CollectionListModel
import org.utbot.cpp.clion.plugin.client.requests.ProjectTargetsRequest
import org.utbot.cpp.clion.plugin.grpc.getProjectTargetsGrpcRequest
import org.utbot.cpp.clion.plugin.listeners.ConnectionStatus
import org.utbot.cpp.clion.plugin.listeners.UTBotEventsListener
import org.utbot.cpp.clion.plugin.listeners.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.settings.UTBotAllProjectSettings
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.getCurrentClient
import org.utbot.cpp.clion.plugin.utils.invokeOnEdt
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.relativize

class UTBotTargetsController(val project: Project) {
    private val settings: UTBotAllProjectSettings
        get() = project.settings

    private val listModel = CollectionListModel(mutableListOf(UTBotTarget.autoTarget))
    private val logger = project.logger

    val targets: List<UTBotTarget>
        get() = listModel.toList()

    init {
        requestTargetsFromServer()
        connectToEvents()
    }

    fun requestTargetsFromServer() {
        val currentClient = project.getCurrentClient()

        ProjectTargetsRequest(
            project,
            getProjectTargetsGrpcRequest(project),
        ) { targetsResponse ->
            invokeOnEdt {
                listModel.apply {
                    val oldTargetList = toList()
                    oldTargetList.addAll(
                        targetsResponse.targetsList.map { projectTarget ->
                            UTBotTarget(projectTarget, project)
                        })
                    listModel.replaceAll(oldTargetList.distinct())
                }
            }
        }.let { targetsRequest ->
            if (!currentClient.isServerAvailable()) {
                logger.error { "Could not request targets from server: server is unavailable!" }
                return
            }
            logger.trace { "Requesting project targets from server!" }
            currentClient.executeRequestIfNotDisposed(targetsRequest)
        }
    }

    private fun addTargetPathIfNotPresent(possiblyNewTargetPath: String) {
        listModel.apply {
            toList().find { utbotTarget -> utbotTarget.path == possiblyNewTargetPath } ?: add(
                UTBotTarget(
                    possiblyNewTargetPath,
                    "custom target",
                    relativize(settings.projectPath, possiblyNewTargetPath)
                )
            )
        }
    }

    fun createTargetsToolWindow(): UTBotTargetsToolWindow {
        return UTBotTargetsToolWindow(listModel, this)
    }

    fun selectionChanged(selectedTarget: UTBotTarget) {
        // when user selects target update model
        settings.storedSettings.targetPath = selectedTarget.path
    }

    fun setTargetByName(targetName: String) {
        val target = targets.find { it.name == targetName } ?: error("No such target!")
        settings.storedSettings.targetPath = target.path
    }

    private fun connectToEvents() {
        project.messageBus.connect().also { connection ->
            // if user specifies some custom target path in settings, it will be added if not already present
            connection.subscribe(
                UTBotSettingsChangedListener.TOPIC,
                UTBotSettingsChangedListener {
                    val possiblyNewTargetPath = settings.storedSettings.targetPath
                    addTargetPathIfNotPresent(possiblyNewTargetPath)
                })
            // when we reconnected to server, the targets should be updated, so we request them from server
            connection.subscribe(
                UTBotEventsListener.CONNECTION_CHANGED_TOPIC,
                object : UTBotEventsListener {
                    override fun onConnectionChange(oldStatus: ConnectionStatus, newStatus: ConnectionStatus) {
                        if (newStatus != oldStatus && newStatus == ConnectionStatus.CONNECTED) {
                            requestTargetsFromServer()
                        }
                    }
                }
            )
        }
    }
}
