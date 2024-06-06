import Foundation
import ARKit
import RealityKit

class WorldView: ARView {
    
    let arConfig = ARWorldTrackingConfiguration()
    let anchor = AnchorEntity(world: SIMD3(x: 0, y: 0, z: 0))
    
    required init(frame: CGRect) {
        // Set/start AR Session to provide camera assistance to new NI Sessions
        arConfig.worldAlignment = .gravity
        arConfig.isCollaborationEnabled = false
        arConfig.userFaceTrackingEnabled = false
        arConfig.initialWorldMap = nil
        
        super.init(frame: .zero)
        
        // Set/start the AR Session. This AR Session will be shared with NISessions
        session = ARSession()
        session.delegate = self
        session.run(arConfig)
        scene.addAnchor(anchor)
        
        isHidden = true
        contentMode = .scaleToFill
        
        // Set up the parent view's constraints
        translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            heightAnchor.constraint(greaterThanOrEqualToConstant: 300.0)
        ])
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
}

// MARK: - `ARSessionDelegate`.

extension WorldView: ARSessionDelegate {
    func sessionShouldAttemptRelocalization(_ session: ARSession) -> Bool {
        return false
    }
}
