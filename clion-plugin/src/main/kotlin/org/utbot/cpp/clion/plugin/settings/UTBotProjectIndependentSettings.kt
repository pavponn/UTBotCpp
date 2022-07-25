package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.components.PersistentStateComponent
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.State
import com.intellij.openapi.components.Storage

/**
 * Settings that are the same for all projects.
 */
@Service
@State(
    name = "UTBotProjectIndependentSettings",
    storages = [Storage("utbot-project-independent-settings.xml")]
)
class UTBotProjectIndependentSettings : PersistentStateComponent<UTBotProjectIndependentSettings.State> {
    data class State(
        var port: Int = UTBotSettings.DEFAULT_PORT,
        var serverName: String = UTBotSettings.DEFAULT_HOST,
    )

    private var myState: State = State()
    override fun getState(): State = myState
    override fun loadState(state: State) {
        myState = state
    }
}