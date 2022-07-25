package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.components.PersistentStateComponent
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.State
import com.intellij.openapi.components.Storage
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.ui.targetsToolWindow.UTBotTarget

/**
 * Settings that are specific for each project.
 */
@Service
@State(
    name = "UTBotProjectDependentSettings",
    storages = [Storage("utbot-project-dependent-settings.xml")]
)
class UTBotProjectDependentSettings(val project: Project) : PersistentStateComponent<UTBotProjectDependentSettings.State> {
    data class State(
        var projectPath: String? = null,
        var buildDirRelativePath: String = "build-utbot",
        var testDirPath: String = "",
        var targetPath: String = UTBotTarget.autoTarget.path,
        var remotePath: String = "",
        var sourceDirs: Set<String> = setOf(),
        var cmakeOptions: List<String> = DEFAULT_CMAKE_OPTIONS,
        var generateForStaticFunctions: Boolean = true,
        var useStubs: Boolean = true,
        var useDeterministicSearcher: Boolean = true,
        var verbose: Boolean = false,
        var timeoutPerFunction: Int = 0,
        var timeoutPerTest: Int = 30
    )

    private var myState = State()
    override fun getState() = myState
    override fun loadState(state: State) {
        myState = state
    }

    companion object {
        val DEFAULT_CMAKE_OPTIONS = listOf("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DCMAKE_EXPORT_LINK_COMMANDS=ON")
    }
}