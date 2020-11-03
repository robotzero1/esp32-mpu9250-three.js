import * as THREE from './build/three.module.js';

import { LightningStrike } from './examples/jsm/geometries/LightningStrike.js';
import { EffectComposer } from './examples/jsm/postprocessing/EffectComposer.js';
import { RenderPass } from './examples/jsm/postprocessing/RenderPass.js';
import { OutlinePass } from './examples/jsm/postprocessing/OutlinePass.js';
import { GLTFLoader } from './examples/jsm/loaders/GLTFLoader.js';

let scene, renderer, composer, camera;

let currentTime = 0;

const clock = new THREE.Clock();
const group = new THREE.Group();
const loader = new GLTFLoader();

init();
animate();

function init() {

	camera = new THREE.PerspectiveCamera(100, window.innerWidth / window.innerHeight, 0.01, 500);
	camera.position.z = 0.7;
	scene = new THREE.Scene();
	scene.canGoBackwardsInTime = true;

	const light = new THREE.PointLight(0xffffcc, 20, 200);
	light.position.set(4, 30, -20);
	scene.add(light);

	const light2 = new THREE.AmbientLight(0x20202A, 10, 100);
	light2.position.set(30, -10, 30);
	scene.add(light2);

	renderer = new THREE.WebGLRenderer();
	renderer.setPixelRatio(window.devicePixelRatio);
	renderer.setSize(window.innerWidth, window.innerHeight);
	renderer.outputEncoding = THREE.sRGBEncoding;

	document.body.appendChild(renderer.domElement);

	composer = new EffectComposer(renderer);

	window.addEventListener('resize', onWindowResize, false);

	scene.timeRate = 1;
	summonLightning();

	loader.load(
		// resource URL
		'glb.glb',
		// called when the resource is loaded
		function(gltf) {
			const object = gltf.scene;
			object.name = "robot";
			object.position.set(0, 0, 0);
			group.add(object);
            		startWebsocket();
		},
		// called while loading is progressing
		function(xhr) {
			console.log((xhr.loaded / xhr.total * 100) + '% loaded');
		},
		// called when loading has errors
		function(error) {
			console.log('An error happened');
		}
	);
	
	scene.add( group );

}

function onWindowResize() {

	camera.aspect = window.innerWidth / window.innerHeight;
	camera.updateProjectionMatrix();

	renderer.setSize(window.innerWidth, window.innerHeight);

	composer.setSize(window.innerWidth, window.innerHeight);

}

function animate() {

	requestAnimationFrame(animate);
			
	render();

}

function render() {

	currentTime += scene.timeRate * clock.getDelta();
	
	if (currentTime < 0) {
		currentTime = 0;
	}
	
	scene.render(currentTime);
}

function moveRobot(qArray) {
  
	//var q = [0.99, -0.99, 0.99, 0.99]; //(x, y, z, w) 
	//var rotation = quaternion.fromArray(q_array);
	//robot.applyQuaternion( rotation );
	const quat1 = new THREE.Quaternion(qArray[1], qArray[2], qArray[3], qArray[0]);
	const quat2 = new THREE.Quaternion(1, 0, 0, 0);
	group.quaternion.multiplyQuaternions(quat1, quat2).conjugate();
  
}


function startWebsocket() {
  
	const WS_URL = "ws://" + window.location.host + "/ws";
	const ws = new WebSocket(WS_URL);

	ws.onopen = () => {
		console.log(`Connected to ${WS_URL}`);
	};
	ws.onmessage = message => {
		if (typeof message.data === "string") {
         		if (message.data.substr(0, 11) == "quaternion:") { // quaternions
				const qArray = message.data.substr(11).split("|");
				moveRobot(qArray);
			}
          		if (message.data.substr(0, 7) == "summon:") { // lightning on/off
				const lightning = message.data.substr(7);
				const thing = scene.getObjectByName( "thor");

				if (lightning == 'on'){
					console.log('summoning');
					thing.visible = true;
				} else {
					console.log('extinguishing');
					thing.visible = false;
				}

			}
		}
	};
  
}

function createOutline(scene, objectsArray, visibleColor) {

	const outlinePass = new OutlinePass(new THREE.Vector2(window.innerWidth, window.innerHeight), scene, camera, objectsArray);
	outlinePass.edgeStrength = 2.5;
	outlinePass.edgeGlow = 0.7;
	outlinePass.edgeThickness = 2.8;
	outlinePass.visibleEdgeColor = visibleColor;
	outlinePass.hiddenEdgeColor.set(0);
	composer.addPass(outlinePass);

	scene.outlineEnabled = true;

	return outlinePass;

}

function summonLightning() {

	// Lights
	scene.lightningColor = new THREE.Color(0xffffff);
	scene.outlineColor = new THREE.Color(0xff0000);

	// Cones
	const coneMesh1 = new THREE.Mesh(new THREE.ConeBufferGeometry(0.1, 0.1, 30, 1, false), new THREE.MeshPhongMaterial({
		color: 0xFFFF00,
		opacity: 0,
		transparent: true
	}));
	coneMesh1.rotation.z = -Math.PI / 2;
	coneMesh1.position.set(-0.3, 0.1, 0.2);
	group.add(coneMesh1);

	const coneMesh2 = new THREE.Mesh(coneMesh1.geometry.clone(), new THREE.MeshPhongMaterial({
		color: 0xFFFF00,
		opacity: 0,
		transparent: true
	}));
	coneMesh2.rotation.z = Math.PI / 2;
	coneMesh2.position.set(0.25, 0.1, 0.2);
	group.add(coneMesh2);

	// Lightning strike
	scene.lightningMaterial = new THREE.MeshBasicMaterial({
		color: scene.lightningColor
	});

	scene.rayParams = {

		sourceOffset: new THREE.Vector3(),
		destOffset: new THREE.Vector3(),
		radius0: 0.01,
		radius1: 0.01,
		minRadius: 0.18,
		radius0Factor: 0.18,
		radius1Factor: 0.2,
		maxIterations: 7,
		isEternal: true,

		timeScale: 0.7,

		propagationTimeFactor: 0.05,
		vanishingTimeFactor: 0.95,
		subrayPeriod: 3.5,
		subrayDutyCycle: 0.6,
		maxSubrayRecursion: 3,
		ramification: 7,
		recursionProbability: 0.6,

		roughness: 1,
		straightness: 0.55

	};

	let lightningStrike;
	let lightningStrikeMesh;
	const outlineMeshArray = [];

	scene.recreateRay = function() {

		lightningStrike = new LightningStrike(scene.rayParams);
		lightningStrikeMesh = new THREE.Mesh(lightningStrike, scene.lightningMaterial);
		lightningStrikeMesh.name = "thor";

		outlineMeshArray.length = 0;
		outlineMeshArray.push(lightningStrikeMesh);
		lightningStrikeMesh.visible = false;
		group.add(lightningStrikeMesh);

	};

	scene.recreateRay();

	// Compose rendering

	composer.passes = [];
	composer.addPass(new RenderPass(scene, camera));
	createOutline(scene, outlineMeshArray, scene.outlineColor);


	scene.render = function(time) {

		// Update ray position

		lightningStrike.rayParameters.sourceOffset.copy(coneMesh1.position);
		lightningStrike.rayParameters.destOffset.copy(coneMesh2.position);

		lightningStrike.update(time);


		if (scene.outlineEnabled) {

			composer.render();

		} else {

			renderer.render(scene, camera);

		}

	};

	return scene;

}