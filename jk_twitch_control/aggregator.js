module.exports = function () {
	this.running = false;

	this.freq = 1.0;
	this.votes = [];
	this.voteOpts = [{
		id: 0,
		name: "forward",
		count: 0,
		vector: {x: 0, y: 0.75},
	}, {
		id: 1,
		name: "backward",
		count: 0,
		vector: {x: 0, y: -0.75},
	}, {
		id: 2,
		name: "left",
		count: 0,
		vector: {x: -1, y: -0.5},
	}, {
		id: 3,
		name: "right",
		count: 0,
		vector: {x: 1, y: 0.5},
	}]

	this.run = function () {
		this.running = true;
		this.reset();
		setInterval(function () {
			this.step();
		}.bind(this), (1.0 / this.freq) * 1000.0);
	}

	this.step = function () {
		var highestVote = {id: -1, name: "", count: 0, vector: {x: 0, y: 0}};
		for (var i = 0; i < this.votes.length; i++) {
			if (this.votes[i].count > highestVote.count) {
				highestVote = this.votes[i];
			}
		}

		if (this.evnt_Tick) {
			this.evnt_Tick(highestVote);
		}

		this.reset();
	}

	this.parse = function (cmd) {
		if (this.running == true) {
			for (var i = 0; i < this.votes.length; i++) {
				if (cmd.toLowerCase() == this.votes[i].name) {
					this.votes[i].count++;
				}
			}
		}
	}

	this.reset = function () {
		this.votes = JSON.parse(JSON.stringify(this.voteOpts));
	}

	this.on = function (evnt, callback) {
		this.evnt_Tick = callback;
	}
}
