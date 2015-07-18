package com.angzangy.video;

import java.io.Serializable;

public class VideoInfo implements Serializable {
	private static final long serialVersionUID = -8046673095564279735L;
	private String filePath;
	private String title;
	private String thumbnailPath;
	private String displayName;
	private String mimeType;
	private long size;
	private long dateModified;

	public void setFilePath(String path) {
		filePath = path;
	}

	public void setTitle(String title) {
		this.title = title;
	}

	public void setDisplayName(String name) {
		displayName = name;
	}

	public void setMimeType(String type) {
		mimeType = type;
	}

	public void setThumbnailPath(String thumbnailPath) {
		this.thumbnailPath = thumbnailPath;
	}

	public String getThumbnailPath() {
		return thumbnailPath;
	}

	public String getFilePath() {
		return filePath;
	}

	public String getTitle() {
		return title;
	}

	public String getDisplayName() {
		return displayName;
	}

	public String getMimeType() {
		return mimeType;
	}

	public long getSize() {
		return size;
	}

	public void setSize(long size) {
		this.size = size;
	}

	public long getDateModified() {
		return dateModified;
	}

	public void setDateModified(long dateModified) {
		this.dateModified = dateModified;
	}

	
	@Override
	public boolean equals(Object o) {
		VideoInfo other = (VideoInfo)o;
		return dateModified > other.getDateModified();
	}

	@Override
	public String toString() {
		return "VideoInfo [filePath=" + filePath + ", title=" + title
				+ ", thumbnailPath=" + thumbnailPath + ", displayName="
				+ displayName + ", mimeType=" + mimeType + ", size=" + size
				+ ", dateModified=" + dateModified + "]";
	}

	
}
